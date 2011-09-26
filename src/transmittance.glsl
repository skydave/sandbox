/**
 * Precomputed Atmospheric Scattering
 * Copyright (c) 2008 INRIA
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holders nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * Author: Eric Bruneton
 */

// computes transmittance table T using Eq (5)

#ifdef _VERTEX_

void main() {
    gl_Position = gl_Vertex;
}

#else

//
// optical depth is the integral over total extinction times density along the ray
// H is the Heightscale which depends on the scattering type (Rayleigh or Mie)
// r is the height
// mu is the view angle
//
float opticalDepth(float H, float r, float mu) {
	float result = 0.0;
	// raysegment length: intersect ray with top/bottom to get the length of the ray-segment and divide it by number of samples
	float dx = limit(r, mu) / float(TRANSMITTANCE_INTEGRAL_SAMPLES);
	// distanced travelled so far
	float xi = 0.0;
	// ??
	float yi = exp(-(r - Rg) / H);
	// raymarching
	for (int i = 1; i <= TRANSMITTANCE_INTEGRAL_SAMPLES; ++i) {
		// get travelled distance
		float xj = float(i) * dx;
		// compute density at current height - density is given by exp(-height/scatteringTypeDependantHeightScale)
		float yj = exp(-(sqrt(r * r + xj * xj + 2.0 * xj * r * mu) - Rg) / H);

		// add current density to last density and ?
		// Trapezoidal rule : ((f(xi) + f(xj))/2.0) * h
		result += (yi + yj) / 2.0 * dx;

		// updated travelled distance
		xi = xj;
		// update last density
		yi = yj;
	}

	//?
	return mu < -sqrt(1.0 - (Rg / r) * (Rg / r)) ? 1e9 : result;
}

void main() {
    float r, muS;
    getTransmittanceRMu(r, muS);
    vec3 depth = betaR * opticalDepth(HR, r, muS) + betaMEx * opticalDepth(HM, r, muS);
    gl_FragColor = vec4(exp(-depth), 0.0); // Eq (5)
}

#endif
