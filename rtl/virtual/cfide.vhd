------------------------------------------------------------------------------
------------------------------------------------------------------------------
--                                                                          --
-- Copyright (c) 2008-2011 Tobias Gubener                                   -- 
-- Subdesign fAMpIGA by TobiFlex                                            --
--                                                                          --
-- This source file is free software: you can redistribute it and/or modify --
-- it under the terms of the GNU General Public License as published        --
-- by the Free Software Foundation, either version 3 of the License, or     --
-- (at your option) any later version.                                      --
--                                                                          --
-- This source file is distributed in the hope that it will be useful,      --
-- but WITHOUT ANY WARRANTY; without even the implied warranty of           --
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            --
-- GNU General Public License for more details.                             --
--                                                                          --
-- You should have received a copy of the GNU General Public License        --
-- along with this program.  If not, see <http://www.gnu.org/licenses/>.    --
--                                                                          --
------------------------------------------------------------------------------
------------------------------------------------------------------------------
-- Modifications by Alastair M. Robinson to work with a cheap 
-- Ebay Cyclone III board.

 
library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.STD_LOGIC_UNSIGNED.all;

entity cfide is
	generic (
		spimux : in boolean
	);
   port ( 
		sysclk	: in std_logic;	
		n_reset	: in std_logic;	

		addr	: in std_logic_vector(31 downto 2);
		d		: in std_logic_vector(31 downto 0);	
		q		: out std_logic_vector(15 downto 0);		
		req 	: in std_logic;
		wr 	: in std_logic;
		ack 	: out std_logic;

		sd_di		: in std_logic;		
		sd_cs 	: out std_logic_vector(7 downto 0);
		sd_clk 	: out std_logic;
		sd_do		: out std_logic;
		sd_dimm	: in std_logic;		--for sdcard
		sd_ack 	: in std_logic; -- indicates that SPI signal has made it to the wire
		debugTxD : out std_logic;
		debugRxD : in std_logic;
		menu_button	: in std_logic:='1';
		scandoubler	: out std_logic;
		
		audio_ena : out std_logic;
		audio_clear : out std_logic;
		audio_buf : in std_logic;
		
		vbl_int	: in std_logic;
		interrupt	: out std_logic;
		c64_keys	: in std_logic_vector(63 downto 0) :=X"FFFFFFFFFFFFFFFF";
		amiga_key	: out std_logic_vector(7 downto 0);
		amiga_key_str	: out std_logic
   );

end cfide;


architecture rtl of cfide is

signal shift: std_logic_vector(9 downto 0);
signal clkgen: std_logic_vector(9 downto 0);
signal shiftout: std_logic;
signal txbusy: std_logic;
signal uart_ld: std_logic;
--signal IO_select : std_logic;
signal platform_select: std_logic;
signal timer_select: std_logic;
signal SPI_select: std_logic;
signal part_in: std_logic_vector(15 downto 0);
signal IOdata: std_logic_vector(15 downto 0);
signal IOcpuena: std_logic;

type support_states is (idle, io_aktion);
signal support_state		: support_states;
signal next_support_state		: support_states;

signal sd_out	: std_logic_vector(15 downto 0);
signal sd_in	: std_logic_vector(15 downto 0);
signal sd_in_shift	: std_logic_vector(15 downto 0);
signal sd_di_in	: std_logic;
signal shiftcnt	: std_logic_vector(13 downto 0);
signal sck		: std_logic;
signal scs		: std_logic_vector(7 downto 0);
signal dscs		: std_logic;
signal SD_busy		: std_logic;
signal spi_div: std_logic_vector(8 downto 0);
signal spi_speed: std_logic_vector(7 downto 0);
signal spi_wait : std_logic;
signal spi_wait_d : std_logic;

signal timecnt: std_logic_vector(15 downto 0);
signal timeprecnt: std_logic_vector(19 downto 0);

signal rs232_select : std_logic;
signal rs232data : std_logic_vector(15 downto 0);

signal audio_q : std_logic_vector(15 downto 0);
signal audio_select : std_logic;

signal interrupt_select : std_logic;
signal interrupt_ena : std_logic;
signal key_select : std_logic;
signal key_q : std_logic_vector(15 downto 0);

begin

q <=	IOdata WHEN rs232_select='1' or SPI_select='1' ELSE
		timecnt when timer_select='1' ELSE 
		audio_q when audio_select='1' else
		part_in;

part_in <=  X"000"&"001"&menu_button; -- Reconfig not currently supported, 32 meg of RAM, menu button.
IOdata <= sd_in;

process(sysclk)
begin
	if rising_edge(sysclk) then
		ack<='0';
		if req='1' then
			if rs232_select='1' or SPI_select='1' then
				ack<=IOcpuena;
			else
--			if timer_select='1' or platform_select='1' or audio_select='1' then
				ack<='1';
			end if;
		end if;
	end if;
end process;


sd_in(15 downto 8) <= (others=>'0');
sd_in(7 downto 0) <= sd_in_shift(7 downto 0);

audio_q<=X"000"&"000"&audio_buf;

SPI_select <= '1' when addr(23)='1' and addr(7 downto 4)=X"E" ELSE '0';
rs232_select <= '1' when addr(23)='1' and addr(7 downto 4)=X"F" ELSE '0';
timer_select <= '1' when addr(23)='1' and addr(7 downto 4)=X"D" ELSE '0';
platform_select <= '1' when addr(23)='1' and addr(7 downto 4)=X"C" ELSE '0';
audio_select <='1' when addr(23)='1' and addr(7 downto 4)=X"B" else '0';
interrupt_select <='1' when addr(23)='1' and addr(7 downto 4)=X"A" else '0';

-- Interrupt handling at ffffffa0
-- Any access to this range will clear the interrupt flag;

process (sysclk,n_reset)
begin
	if n_reset='0' then
		interrupt<='0';
		interrupt_ena<='0';
	elsif rising_edge(sysclk) then
		if vbl_int='1' then
			interrupt<=interrupt_ena;
		end if;
		if interrupt_select='1' and req='1' then
			interrupt<='0';
			if  wr='1' then
				interrupt_ena<=d(0);
			end if;
		end if;
	end if;
end process;


---------------------------------
-- Platform specific registers --
---------------------------------

process(sysclk,n_reset)
begin
	if rising_edge(sysclk) then
		if req='1' and wr='1' then
		
			if platform_select='1' then	-- Write to platform registers
				scandoubler<=d(0);
			end if;
			
			if audio_select='1' then
				audio_clear<=d(1);
				audio_ena<=d(0);
			end if;
			
		end if;
	end if;
end process;


-----------------------------------------------------------------
-- Support States
-----------------------------------------------------------------
process(sysclk, shift)
begin
  	IF sysclk'event AND sysclk = '1' THEN
		support_state <= idle;
		uart_ld <= '0';
		IOcpuena <= '0';
		CASE support_state IS
			WHEN idle => 
				IF rs232_select='1' AND req='1' and wr='1' THEN
					IF txbusy='0' THEN
						uart_ld <= '1';
						support_state <= io_aktion;
						IOcpuena <= '1';
					END IF;	
				ELSIF SPI_select='1' and req='1' THEN
					IF SD_busy='0' THEN
						support_state <= io_aktion;
						IOcpuena <= '1';
					END IF;
				END IF;
					
			WHEN io_aktion =>
				if req='0' then
					support_state <= idle;
				end if;
				
			WHEN OTHERS => 
				support_state <= idle;
		END CASE;
	END IF;	
end process; 

-----------------------------------------------------------------
-- SPI-Interface
-----------------------------------------------------------------	
	sd_cs <= NOT scs;
	sd_clk <= NOT sck;
	sd_do <= sd_out(15);
	SD_busy <= shiftcnt(13);
	
	PROCESS (sysclk, n_reset, scs, sd_di, sd_dimm) BEGIN
		IF scs(1)='0' THEN
			sd_di_in <= sd_di;
		ELSE	
			sd_di_in <= sd_dimm;
		END IF;
		IF n_reset ='0' THEN 
			shiftcnt <= (OTHERS => '0');
			spi_div <= (OTHERS => '0');
			scs <= (OTHERS => '0');
			sck <= '0';
			spi_speed <= "00000000";
			dscs <= '0';
			spi_wait <= '0';
		ELSIF (sysclk'event AND sysclk='1') THEN

		spi_wait_d<=spi_wait;
		
		if spi_wait_d='1' and sd_ack='1' then -- Unpause SPI as soon as the IO controller has written to the MUX
			spi_wait<='0';
		end if;

		IF SPI_select='1' AND req='1' and wr='1' AND SD_busy='0' THEN	 --SD write
			case addr(3 downto 2) is				
				when "10" => -- 8
					spi_speed <= d(7 downto 0);
				when "01" => -- 4
					scs(0) <= not d(0);
					IF d(7)='1' THEN
						scs(7) <= not d(0);
					END IF;
					IF d(6)='1' THEN
						scs(6) <= not d(0);
					END IF;
					IF d(5)='1' THEN
						scs(5) <= not d(0);
					END IF;
					IF d(4)='1' THEN
						scs(4) <= not d(0);
					END IF;
					IF d(3)='1' THEN
						scs(3) <= not d(0);
					END IF;
					IF d(2)='1' THEN
						scs(2) <= not d(0);
					END IF;
					IF d(1)='1' THEN
						scs(1) <= not d(0);
					END IF;
				when "00" => -- 0
--						ELSE							--DA4000
					if scs(1)='1' THEN -- Wait for io component to propagate signals.
						spi_wait<='1'; -- Only wait if SPI needs to go through the MUX
						if spimux = true then
							spi_div(8 downto 1) <= spi_speed+4;
						else
							spi_div(8 downto 1) <= spi_speed;
						end if;
					else
						spi_div(8 downto 1) <= spi_speed;
					end if;
					IF scs(6)='1' THEN		-- SPI direkt Mode
						shiftcnt <= "10111111111111";
						sd_out <= X"FFFF";
					ELSE
						shiftcnt <= "10000000000111";
						sd_out(15 downto 8) <= d(7 downto 0);
					END IF;
					sck <= '1';
				when others =>
					null;
			end case;
		ELSE
			IF spi_div="0000000000" THEN
				if scs(1)='1' THEN -- Wait for io component to propagate signals.
					spi_wait<='1'; -- Only wait if SPI needs to go through the MUX
					if spimux=true then
						spi_div(8 downto 1) <= spi_speed+4;
					else
						spi_div(8 downto 1) <= spi_speed;
					end if;
				else
					spi_div(8 downto 1) <= spi_speed;
				end if;
				IF SD_busy='1' THEN
					IF sck='0' THEN
						IF shiftcnt(12 downto 0)/="0000000000000" THEN
							sck <='1';
						END IF;
						shiftcnt <= shiftcnt-1;
						sd_out <= sd_out(14 downto 0)&'1';
					ELSE
						sck <='0';
						sd_in_shift <= sd_in_shift(14 downto 0)&sd_di_in;
					END IF;
				END IF;
			ELSif spi_wait='0' then
				spi_div <= spi_div-1;
			END IF;
		END IF;

		END IF;		
	END PROCESS;

-----------------------------------------------------------------
-- Simple UART only TxD
-----------------------------------------------------------------
debugTxD <= not shiftout;
process(n_reset, sysclk, shift)
begin
	if shift="0000000000" then
		txbusy <= '0';
	else
		txbusy <= '1';
	end if;

	if n_reset='0' then
		shiftout <= '0';
		shift <= "0000000000"; 
	elsif rising_edge(sysclk) then
		if uart_ld = '1' then
			shift <=  '1' & d(7 downto 0) & '0';			--STOP,MSB...LSB, START
		end if;
		if clkgen/=0 then
			clkgen <= clkgen-1;
		else	
			clkgen <= "1111011001";--985;		--113.5MHz/115200
			shiftout <= not shift(0) and txbusy;
			shift <=  '0' & shift(9 downto 1);
		end if;
	end if;
end process; 


-----------------------------------------------------------------
-- timer
-----------------------------------------------------------------
process(sysclk)
begin
  	IF rising_edge(sysclk) THEN
		IF timeprecnt=0 THEN
			timeprecnt <= X"3808F";
			timecnt <= timecnt+1;
		ELSE
			timeprecnt <= timeprecnt-1;
		END IF;
	end if;
end process; 

end;  

