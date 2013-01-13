/*
 * mlt_movit_input.h
 * Copyright (C) 2013 Dan Dennedy <dan@dennedy.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef MLT_MOVIT_INPUT_H
#define MLT_MOVIT_INPUT_H

#include "movit/flat_input.h"
#include "movit/ycbcr_input.h"
#include "movit/effect_chain.h"

class MltInput : public Input
{
public:
	MltInput(unsigned width, unsigned height)
		: width(width)
		, height(height)
		, output_linear_gamma(false)
		, needs_mipmaps(false)
		, input(0)
		, isRGB(true)
	{
		register_int("output_linear_gamma", &output_linear_gamma);
		register_int("needs_mipmaps", &needs_mipmaps);
	}
	~MltInput() {
		delete input;
	}

	// Effect overrides
	std::string effect_type_id() const { return "MltInput"; }
	std::string output_fragment_shader() {
		assert(input);
		return input->output_fragment_shader();
	}
	void set_gl_state(GLuint glsl_program_num, const std::string& prefix, unsigned *sampler_num) {
		assert(input);
		input->set_gl_state(glsl_program_num, prefix, sampler_num);
	}

	// Input ovverrides
	void finalize() {
		assert(input);
		input->set_int("output_linear_gamma", output_linear_gamma);
		input->set_int("needs_mipmaps", needs_mipmaps);
		input->finalize();
	}
	bool can_output_linear_gamma() const {
		assert(input);
		return input->can_output_linear_gamma();
	}
	unsigned get_width() const {
		return width;
	}
	unsigned get_height() const {
		return height;
	}
	Colorspace get_color_space() const {
		assert(input);
		return input->get_color_space();
	}
	GammaCurve get_gamma_curve() const {
		assert(input);
		return input->get_gamma_curve();
	}

	// Custom methods
	void useFlatInput(EffectChain* chain) {
		if (!input) {
			ImageFormat image_format;
			image_format.color_space = COLORSPACE_sRGB;
			image_format.gamma_curve = GAMMA_sRGB;
			input = new FlatInput(image_format, FORMAT_RGBA, GL_UNSIGNED_BYTE, width, height);
			chain->add_output(image_format);
		}
	}
	void useYCbCrInput(EffectChain* chain, const ImageFormat& image_format, const YCbCrFormat& ycbcr_format) {
		if (!input) {
			input = new YCbCrInput(image_format, ycbcr_format, width, height);
			chain->add_output(image_format);
			isRGB = false;
		}
	}
	void set_pixel_data(const unsigned char* data) {
		assert(input);
		if (isRGB) {
			FlatInput* flat = (FlatInput*) input;
			flat->set_pixel_data(data);
		} else {
			YCbCrInput* ycbcr = (YCbCrInput*) input;
			// Assumed to be yuv422p
			ycbcr->set_pixel_data(0, data);
			ycbcr->set_pixel_data(1, &data[width * height]);
			ycbcr->set_pixel_data(2, &data[width * height * 3/2]);
		}
	}

private:
	unsigned width, height;
	int output_linear_gamma, needs_mipmaps;
	Input *input;
	bool isRGB;
};

#endif // MLT_MOVIT_INPUT_H