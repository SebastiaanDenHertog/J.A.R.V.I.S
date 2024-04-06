/* Audio Library for Teensy 3.X
 * Copyright (c) 2017, Paul Stoffregen, paul@pjrc.com
 *
 * Development of this audio library was funded by PJRC.COM, LLC by sales of
 * Teensy and Audio Adaptor boards.  Please support PJRC's efforts to develop
 * open source software by purchasing Teensy or other PJRC products.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice, development funding notice, and this permission
 * notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef _input_tdm_h_
#define _input_tdm_h_

#include "AudioStream.h"
#include "DMAChannel.h"

class AudioInputTDM : public AudioStream
{
public:
	AudioInputTDM(void) : AudioStream(0, NULL) { begin(); }
	virtual void update(void);
	void begin(void);
	static DMAChannel dma;
 	static bool update_responsibility;
	static void isr(void);
	void pause(void);
	void resume(void);
protected:	
 	AudioInputTDM(int dummy): AudioStream(0, NULL) {} // to be used only inside AudioInputI2Sslave !!
private:
	static audio_block_t *block_incoming[8];
};

class AudioInputTDMslave : public AudioInputTDM
{
protected:
public:
	static int cnls;
	AudioInputTDMslave(int channels) : AudioInputTDM(0) { begin(channels); }	
	void begin(int channels);
	friend void dma_ch1_isr(void);
};
#endif
