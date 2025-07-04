# LITTLE-BM

ABOUT LITTLE BM

	Little BM is a simple DIY portable microcontroller based FM synthesizer designed to be simple, cheap and easy to build, with a variety of sounds to play with. 
	
	It can be powered by a 9 volt battery or a 9v DC psu.

	Little BM uses bitwise (OR and XOR)frequency modulation with a sync mode to produce a wide variety of sounds. 
	
	It has a simple LF0 with frequency control and 4 wavforms at 2 resolution rates (lower res waves for faster frequencies). 
	
	It has tremelo control with waveform inversion and vibrato with 4 bit PAM (pulse amplitude modulation) linear output. 
	
	The design has a line level/headphone output and a speaker output powered by a lm386 audio amplifier. 
	
	The Little BM is designed to be a portable knob controlled drone synth with a built in ldr for use as a optic_theramin.


LITTLE BM CONTROLS CONNECTIONS DEACRIPTION

	1)	POWER INPUT: 9v DC through .33 mm phono jack. 
		
					It can be powered with 7v-12v DC but is designed to run off
						of a9v battery or 9v psu.

					The board can be powered with the microcontrollers usb port
						if not using the amplified speaker output. Poweing the
						amplifier off the microcontroller would likeley damage
						the microcontroller, lm386, other componants and other
						equipment that you may be plugged in to.

	2)	AUXILLARY/HEAD PHONE AUDIO OUTPUt:	.33 mm jack for mono audio output.
						this will work with headphones and auxillary line level
						inputs.

	3)	SPEAKER OUTPUT:	Amplified audio output designed to power a 4ohm to 
						8ohm speaker. 

	4)	POWER SWITCH:	Toggles external power on/off.

	5)	AUXILLARY/HEADPHONE VOLUME CONTROL:	Controls aux/headphone output
						volume. Aways start with the volume low and work up to a
						comftorable audible level. High volume levels may
						damage hearing or equipment.

	6)	SPEAKER OUTPUT GAIN: Controls volume level on spekaer output.
						Keep potentiometer below 20% for clean
						sound. Above 20% will produce distortion.

	7)	AUXILLARY/SPEAKER OUTPUT SWITCH:	Toggles between aux/headphone and
						speaker output.

	8)	PITCH CONTROL:	controls the carrier frequency. 5 octave note range.
						use to tune optic-theramin when ldr is switched on.

	9)	FM CONTROL:		controls the modulation frequency. 2 octave range.
						modulator frequency is linked to the carriers frequency.
						{fm_frequency}+={carrier_frequency}

	10)	TREMELO DEPTH:	Controls tremelo depth level divided down from lfo shape
						and added to pitch (carrier frequency).

	11)	LFO SPEED:		Controls the frequency of the LFO.

	12)	LFO SHAPE:		Controls LFO shape output.
							shapes 0-3...square, triangle, saw, ramp @ 16 byte
							samples. (speed*1)
							shapes 4-7...square, triangle, saw, ramp @ 8 byte
							samples. (speed*2)

	13)	VIBRATO DEPTH:	Controls vibrato depth level divided down from lfo shape
						and subtracted from peak gain.

	14) OR/XOR MODE SWITCH:	Toggles or/xor fm modes.

	15) FM SYNC SWITCH: Toggles frequency synchronization.

	16)	TREMELO INVERSION SWITCH: inverts the tremelo from positive to negative.

	17)	OPTIC THERAMIN SWITCH:	switches the LDR on and off for the use as a 
						optic-theramin.

PARTS LIST
	      arduino nano, uno or compatable clones
              lm386 audio power amplifier
              8 leg ic socket
              -8 10k potentiometers
              -6 spdt switches
              -3 .33 mm jacks
              -prototyping board
              -wire
              -220 uf capacitor
              -22 uf capacitor
              -10 uf capacitor
              -2* 100 nf capacitors
              -50 nf capacitors
              -6* 470 ohm resistors
              -ldr              
              -2* 10 ohm resistor
              -100 ohm resistor
              -5* 2k resistors
              -4* 1k resistors
              -47k resistor



