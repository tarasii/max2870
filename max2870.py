import wiringpi
from wiringpi import GPIO
import time


class max2870:
	
	init_tx = [
		0x00400005, 
		0x00000004, 
		0x00000003, 
		0x00000002, 
		0x00000001, 
		0x00000000 
	]	
	
	def __init__(self, ce_pin, le_pin, F_REF, MOD = 4000, R_DIV = 2):
		self.ce_pin = ce_pin
		self.le_pin = le_pin
		self.F_REF = F_REF
		self.M = MOD
		self.R = R_DIV
		
		
		wiringpi.wiringPiSPISetupMode(0, 0, 500000, 0)

		wiringpi.wiringPiSetup()

		wiringpi.pinMode(self.ce_pin, GPIO.OUTPUT)
		wiringpi.pinMode(self.le_pin, GPIO.OUTPUT)

		wiringpi.digitalWrite(self.ce_pin, GPIO.HIGH)
		wiringpi.digitalWrite(self.le_pin, GPIO.LOW)

		self.write_reg(self.init_tx[0])
		time.sleep(0.020)
		self.write_reg(self.init_tx[1])
		self.write_reg(self.init_tx[2])
		self.write_reg(self.init_tx[3])
		self.write_reg(self.init_tx[4])
		self.write_reg(self.init_tx[5])

		time.sleep(0.1)
		self.write_reg(self.init_tx[0])
		self.write_reg(self.init_tx[1])
		self.write_reg(self.init_tx[2])
		self.write_reg(self.init_tx[3])
		self.write_reg(self.init_tx[4])
		self.write_reg(self.init_tx[5])
		
		self.calc_f_pfd()
		
		self.P      = 1
		self.cpoc   = 0
		self.cpt    = 0
		self.cpl    = 0
		
		self.rdiv2  = 0
		self.sdn    = 0
		self.mux    = 0
		self.dbr    = 0
		self.pdp    = 1
		self.rst    = 0
		self.tri    = 0
		self.shdn   = 0
		self.tri    = 0
		self.ldp    = 0
		self.ldf    = 0
		self.cp     = 0
		self.reg4p  = 0
		self.vas_shdn = 0
		self.vco    = 0
		self.retune = 0
		self.cdm    = 0
		self.int    = 0
		self.bdiv   = 0
		self.bpwr   = 0
		self.rfb_en = 0
		self.apwr   = 3
		self.rfa_en = 1
		self.fb     = 1 #N counter mode = fundamental

	def pulse_le(self):
		wiringpi.digitalWrite(self.le_pin, GPIO.HIGH)
		time.sleep(0.0001)
		wiringpi.digitalWrite(self.le_pin, GPIO.LOW)
		time.sleep(0.0001)
	
	def write_reg(self, x):
		r = [(x & 0xff000000) >> 24,
			 (x & 0x00ff0000) >> 16,
			 (x & 0x0000ff00) >>  8,
			 (x & 0x000000ff)]
		revlen, recvData = wiringpi.wiringPiSPIDataRW(0, bytes(r))
		self.pulse_le()

	def get_diva(self, freq_kHz):
		self.diva = 0
		while (freq_kHz * 2**self.diva < 3000000):
			self.diva = self.diva + 1;
		
		return self.diva
		
	def calc_f_pfd(self):
		self.f_pfd = self.F_REF / self.R;

		self.bs = int(self.f_pfd / 50000);
		if (self.bs > 1023):
			self.bs = 1023;
	
		self.cdiv = int(self.f_pfd / 100000);

		self.lds = 0;
		if (self.f_pfd > 32000000) :
			self.lds = 1;

	def prep_regs(self):
		# --- Формуємо регістри ---
		self.R0 = (self.int  << 31) | \
				  (self.N    << 15) | \
				  (self.FRAC <<  3) | \
				  0;
				  
		self.R1 = (self.cpoc  << 31) | \
				  (self.cpl   << 29) | \
				  (self.cpt   << 27) | \
				  (self.P     << 15) | \
				  (self.M     <<  3) | \
				  1;

		# R2 – включає PFD, R divider
		self.R2 = (self.lds   << 31) | \
				  (self.sdn   << 29) | \
				  (self.mux   << 26) | \
				  (self.dbr   << 25) | \
				  (self.rdiv2 << 24) | \
				  (self.R     << 14) | \
				  (self.reg4p << 13) | \
				  (self.cp    <<  9) | \
				  (self.ldf   <<  8) | \
				  (self.ldp   <<  7) | \
				  (self.pdp   <<  6) | \
				  (self.shdn  <<  5) | \
				  (self.tri   <<  4) | \
				  (self.rst   <<  3) | \
				  2;

		self.R3 = (self.vco      << 26) | \
				  (self.vas_shdn << 25) | \
				  (self.retune   << 24) | \
				  (self.cdm      << 15) | \
				  (self.cdiv     <<  3) | \
				  3;  

		self.R4 = (0x18 << 26) | \
				  (((self.bs >>  8) & 0x03) << 24) | \
				  (self.fb        << 23) | \
				  (self.diva      << 20) | \
				  ((self.bs & 0xff) << 12) | \
				  (self.bdiv      <<  9) | \
				  (self.rfb_en    <<  8) | \
				  (self.bpwr      <<  6) | \
				  (self.rfa_en    <<  5) | \
				  (self.apwr      <<  3) | \
				  4
				   
		self.R5 = 0x400005;  # стандартний

	def setFrequency(self, freq_kHz):

		if (freq_kHz < 23500): return 0
		if (freq_kHz > 6000000): return 0 

		self.get_diva(freq_kHz)

		ratio = freq_kHz * 1000 * 2**self.diva / self.f_pfd
		self.N = int(ratio);
		self.FRAC = int((ratio - self.N) * self.M);
		#print(f"f_pfd:{self.f_pfd}; diva:{diva}; ratio:{ratio}; INT:{self.INT}; FRAC:{self.FRAC}; bs:{bs}; cdiv:{cdiv}");
	
		self.prep_regs()

		# --- Відправляємо регістри ---
		self.write_reg(self.R5)
		self.write_reg(self.R4)
		self.write_reg(self.R3)
		self.write_reg(self.R2)
		self.write_reg(self.R1)
		self.write_reg(self.R0)

		freq_res = self.f_pfd * (self.N + self.FRAC / self.M) / 2**self.diva
		
		return freq_res / 1000

	
	def __str__(self):
		return f"{self.R0:08X}\n{self.R1:08X}\n{self.R2:08X}\n{self.R3:08X}\n{self.R4:08X}\n{self.R5:08X}\n";
	




