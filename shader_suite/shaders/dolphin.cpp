#include "dolphin.h"

/**
 Shaders from the Dolphin emulator.
 https://github.com/dolphin-emu/dolphin
 Extracted version (generated on the fly in practice):
 https://gist.github.com/phire/25181a9bfd957ac68ea8c74afdd9e9e1
 */

csl::glsl::vert_420::Shader dolphin_ubershader_vertex()
{
	using namespace csl::glsl::vert_420;
	using namespace csl::swizzles::all;

	Shader shader;

	CSL_STRUCT(Light,
		(ivec4, color),
		(vec4, cosatt),
		(vec4, distatt),
		(vec4, pos),
		(vec4, dir)
	);

	CSL_UNNAMED_INTERFACE_BLOCK(
		(Layout<Binding<2>, Std140>, Uniform),
		VSBLock,
		(Uint, components),
		(Uint, xfmem_dualTexInfo),
		(Uint, xfmem_numColorChans),
		((Qualify<vec4, Array<6>>), cpnmtx),
		((Qualify<vec4, Array<4>>), cproj),
		((Qualify<ivec4, Array<6>>), cmtrl),
		((Qualify<Light, Array<8>>), clights),
		((Qualify<vec4, Array<24>>), ctexmtx),
		((Qualify<vec4, Array<64>>), ctrmtx),
		((Qualify<vec4, Array<32>>), cnmtx),
		((Qualify<vec4, Array<64>>), cpostmtx),
		(vec4, cpixelcenter),
		(vec2, cviewport),
		((Qualify<uvec4, Array<8>>), xfmem_pack1)
	);

	CSL_STRUCT(VS_OUTPUT,
		(vec4, pos),
		(vec4, colors_0),
		(vec4, colors_1),
		(vec3, tex0),
		(vec3, tex1),
		(vec4, clipPos),
		(Float, clipDist0),
		(Float, clipDist1)
	);

	auto CalculateLighting = define_function<ivec4>("CalculateLighting",
		[&](Uint index = "index", Uint attnfunc = "attnfunc", Uint diffusefunc = "diffusefunc",
			vec3 pos = "pos", vec3 normal = "normal")
	{
		vec3 ldir("ldir"), h("h"), cosAttn("cosAttn"), distAttn("distAttn");
		Float dist("dist"), dist2("dist2"), attn("attn");

		CSL_SWITCH(attnfunc)
		{
			// LIGNTATTN_NONE
			CSL_CASE(0u) : { }

			// LIGHTATTN_DIR
			CSL_CASE(2u) : {
				ldir = normalize(clights[index].pos[x, y, z] - pos[x, y, z]);
				attn = 1.0;
				CSL_IF(length(ldir) == 0.0)
					ldir = normal;
				CSL_BREAK;
			}

			// LIGHTATTN_SPEC
			CSL_CASE(1u) : {
				ldir = normalize(clights[index].pos[x, y, z] - pos[x, y, z]);
				attn = CSL_TERNARY(dot(normal, ldir) >= 0.0, max(0.0, dot(normal, clights[index].dir[x, y, z])), 0.0);
				cosAttn = clights[index].cosatt[x, y, z];
				CSL_IF(diffusefunc == 0u) // LIGHTDIF_NONE
					distAttn = clights[index].distatt[x, y, z];
				CSL_ELSE
					distAttn = normalize(clights[index].distatt[x, y, z]);
				attn = max(0.0, dot(cosAttn, vec3(1.0, attn, attn * attn))) / dot(distAttn, vec3(1.0, attn, attn * attn));
				CSL_BREAK;
			}

			// LIGHTATTN_SPOT
			CSL_CASE(3u) : {
				ldir = clights[index].pos[x, y, z] - pos[x, y, z];
				dist2 = dot(ldir, ldir);
				dist = sqrt(dist2);
				ldir = ldir / dist;
				attn = max(0.0, dot(ldir, clights[index].dir[x, y, z]));
				attn = max(0.0, clights[index].cosatt[x] + clights[index].cosatt[y] * attn + clights[index].cosatt[z] * attn * attn) / dot(clights[index].distatt[x, y, z], vec3(1.0, dist, dist2));
				CSL_BREAK;
			}

		CSL_DEFAULT: {
			attn = 1.0;
			ldir = normal;
			CSL_BREAK;
			}
		}

		CSL_SWITCH(diffusefunc) {
			// LIGHTDIF_NONE
			CSL_CASE(0u) : {
				CSL_RETURN(ivec4(round(attn* vec4(clights[index].color))));
			}

			// LIGHTDIF_SIGN
			CSL_CASE(1u) : {
				CSL_RETURN(ivec4(round(attn* dot(ldir, normal)* vec4(clights[index].color))));
			}

			CSL_CASE(2u) : {
				CSL_RETURN(ivec4(round(attn* max(0.0, dot(ldir, normal))* vec4(clights[index].color))));
			}

		CSL_DEFAULT: {
			CSL_RETURN(ivec4(0, 0, 0, 0));
			}
		}

	});

	Qualify<vec4, In> rawpos("rawpos");
	Qualify<uvec4, In> posmtx("posmtx");
	Qualify<vec3, In> rawnorm0("rawnorm0"), rawnorm1("rawnorm1"), rawnorm2("rawnorm2");
	Qualify<vec4, In> rawcolor0("rawcolor0"), rawcolor1("rawcolor1");
	Qualify<vec3, In> rawtex0("rawtex0"), rawtex1("rawtex1"), rawtex2("rawtex2"), rawtex3("rawtex3"), rawtex4("rawtex4"), rawtex5("rawtex5"), rawtex6("rawtex6"), rawtex7("rawtex7");

	CSL_INTERFACE_BLOCK(Out, VertexData, vs,
		(vec4, pos),
		(vec4, colors_0),
		(vec4, colors_1),
		(vec3, tex0),
		(vec3, tex1),
		(vec4, clipPos),
		(Float, clipDist0),
		(Float, clipDist1)
	);

	shader.main([&] {
		VS_OUTPUT o("o");

		// Position matrix
		vec4 P0, P1, P2;

		// Normal matrix
		vec3 N0, N1, N2;

		CSL_IF((components & 2u) != 0u) {// VB_HAS_POSMTXIDX
			// Vertex format has a per-vertex matrix
			Int posidx = Int(posmtx[r]);
			P0 = ctrmtx[posidx];
			P1 = ctrmtx[posidx + 1];
			P2 = ctrmtx[posidx + 2];

			Int normidx = CSL_TERNARY(posidx >= 32, (posidx - 32), posidx);
			N0 = cnmtx[normidx][x, y, z];
			N1 = cnmtx[normidx + 1][x, y, z];
			N2 = cnmtx[normidx + 2][x, y, z];
		} CSL_ELSE{
			// One shared matrix
			P0 = cpnmtx[0];
			P1 = cpnmtx[1];
			P2 = cpnmtx[2];
			N0 = cpnmtx[3][x, y, z];
			N1 = cpnmtx[4][x, y, z];
			N2 = cpnmtx[5][x, y, z];
		}

		vec4 pos = vec4(dot(P0, rawpos), dot(P1, rawpos), dot(P2, rawpos), 1.0);
		o.pos = vec4(dot(cproj[0], pos), dot(cproj[1], pos), dot(cproj[2], pos), dot(cproj[3], pos));

		// Only the first normal gets normalized (TODO: why?)
		vec3 _norm0 = vec3(0.0, 0.0, 0.0);
		CSL_IF((components & 1024u) != 0u) // VB_HAS_NRM0
			_norm0 = normalize(vec3(dot(N0, rawnorm0), dot(N1, rawnorm0), dot(N2, rawnorm0)));

		vec3 _norm1 = vec3(0.0, 0.0, 0.0);
		CSL_IF((components & 2048u) != 0u) // VB_HAS_NRM1
			_norm1 = vec3(dot(N0, rawnorm1), dot(N1, rawnorm1), dot(N2, rawnorm1));

		vec3 _norm2 = vec3(0.0, 0.0, 0.0);
		CSL_IF((components & 4096u) != 0u) // VB_HAS_NRM2
			_norm2 = vec3(dot(N0, rawnorm2), dot(N1, rawnorm2), dot(N2, rawnorm2));

		// Lighting
		CSL_FOR(Uint chan = Uint(0u) << "chan"; chan < xfmem_numColorChans; chan++) {
			Uint colorreg = (xfmem_pack1[(chan)][z]);
			Uint alphareg = (xfmem_pack1[(chan)][w]);
			ivec4 mat = cmtrl[chan + 2u];
			ivec4 lacc = ivec4(255, 255, 255, 255);

			CSL_IF(bitfieldExtract(colorreg, 0, 1) != 0u) {
				CSL_IF((components & (8192u << chan)) != 0u) // VB_HAS_COL0
					mat[x, y, z] = ivec3(round(CSL_TERNARY(chan == 0u, rawcolor0[x, y, z], rawcolor1[x, y, z]) * 255.0));
				CSL_ELSE_IF((components & 8192u) != 0u) // VB_HAS_COLO0
					mat[x, y, z] = ivec3(round(rawcolor0[x, y, z] * 255.0));
				CSL_ELSE
					mat[x, y, z] = ivec3(255, 255, 255);
			}

			CSL_IF(bitfieldExtract(alphareg, 0, 1) != 0u) {
				CSL_IF((components & (8192u << chan)) != 0u) // VB_HAS_COL0
					mat[w] = Int(round(CSL_TERNARY(chan == 0u, rawcolor0[w], rawcolor1[w]) * 255.0));
				CSL_ELSE_IF((components & 8192u) != 0u) // VB_HAS_COLO0
					mat[w] = Int(round(rawcolor0[w] * 255.0));
				CSL_ELSE
					mat[w] = 255;
			} CSL_ELSE{
				mat[w] = cmtrl[chan + 2u][w];
			}

				CSL_IF(bitfieldExtract(colorreg, 1, 1) != 0u) {
				CSL_IF(bitfieldExtract(colorreg, 6, 1) != 0u) {
					CSL_IF((components & (8192u << chan)) != 0u) // VB_HAS_COL0
						lacc[x, y, z] = ivec3(round(CSL_TERNARY(chan == 0u, rawcolor0[x, y, z], rawcolor1[x, y, z]) * 255.0));
					CSL_ELSE_IF((components & 8192u) != 0u) // VB_HAS_COLO0
						lacc[x, y, z] = ivec3(round(rawcolor0[x, y, z] * 255.0));
					CSL_ELSE
						lacc[x, y, z] = ivec3(255, 255, 255);
				} CSL_ELSE{
					lacc[x, y, z] = cmtrl[chan][x, y, z];
				}

				Uint light_mask = bitfieldExtract(colorreg, 2, 4) | (bitfieldExtract(colorreg, 11, 4) << 4u);
				Uint attnfunc = bitfieldExtract(colorreg, 9, 2);
				Uint diffusefunc = bitfieldExtract(colorreg, 7, 2);
				CSL_FOR(Uint light_index = 0u; light_index < 8u; light_index++) {
					CSL_IF((light_mask & (1u << light_index)) != 0u)
						lacc[x, y, z] += CalculateLighting(light_index, attnfunc, diffusefunc, pos[x, y, z], _norm0)[x, y, z];
				}
			}

			CSL_IF(bitfieldExtract(alphareg, 1, 1) != 0u) {
				CSL_IF(bitfieldExtract(alphareg, 6, 1) != 0u) {
					CSL_IF((components & (8192u << chan)) != 0u) // VB_HAS_COL0
						lacc[w] = Int(round(CSL_TERNARY(chan == 0u, rawcolor0[w], rawcolor1[w]) * 255.0));
					CSL_ELSE_IF((components & 8192u) != 0u) // VB_HAS_COLO0
						lacc[w] = Int(round(rawcolor0[w] * 255.0));
					CSL_ELSE
						lacc[w] = 255;
				} CSL_ELSE{
					lacc[w] = cmtrl[chan][w];
				}

				Uint light_mask = bitfieldExtract(alphareg, 2, 4) | (bitfieldExtract(alphareg, 11, 4) << 4u);
				Uint attnfunc = bitfieldExtract(alphareg, 9, 2);
				Uint diffusefunc = bitfieldExtract(alphareg, 7, 2);
				CSL_FOR(Uint light_index = 0u; light_index < 8u; light_index++) {
					CSL_IF((light_mask & (1u << light_index)) != 0u)
						lacc[w] += CalculateLighting(light_index, attnfunc, diffusefunc, pos[x, y, z], _norm0)[w];
				}
			}

			lacc = clamp(lacc, 0, 255);

			// Hopefully GPUs that can support dynamic indexing will optimize this.
			vec4 lit_color = vec4((mat * (lacc + (lacc >> 7))) >> 8) / 255.0;
			CSL_SWITCH(chan) {
				CSL_CASE(0u) : { o.colors_0 = lit_color; CSL_BREAK; }
				CSL_CASE(1u) : { o.colors_1 = lit_color; CSL_BREAK; }
			CSL_DEFAULT: { }
			}
		}

		CSL_IF(xfmem_numColorChans < 2u && (components & 16384u) == 0u)
			o.colors_1 = o.colors_0;

		o.tex0 = vec3(0.0, 0.0, 0.0);
		o.tex1 = vec3(0.0, 0.0, 0.0);

		CSL_FOR(Uint texgen = 0u; texgen < 2u; texgen++) {
			// Texcoord transforms
			vec4 coord = vec4(0.0, 0.0, 1.0, 1.0);
			Uint texMtxInfo = (xfmem_pack1[(texgen)][x]);

			CSL_SWITCH(bitfieldExtract(texMtxInfo, 7, 5)) {
				CSL_CASE(0u) : // XF_SRCGEOM_INROW
					coord[x, y, z] = rawpos[x, y, z]; CSL_BREAK;

				CSL_CASE(1u) : // XF_SRCNORMAL_INROW
					coord[x, y, z] = CSL_TERNARY((components & 1024u /* VB_HAS_NRM0 */) != 0u, rawnorm0[x, y, z], coord[x, y, z]); CSL_BREAK;

				CSL_CASE(3u) : // XF_SRCBINORMAL_T_INROW
					coord[x, y, z] = CSL_TERNARY((components & 2048u /* VB_HAS_NRM1 */) != 0u, rawnorm1[x, y, z], coord[x, y, z]); CSL_BREAK;

				CSL_CASE(4u) : // XF_SRCBINORMAL_B_INROW
					coord[x, y, z] = CSL_TERNARY((components & 4096u /* VB_HAS_NRM2 */) != 0u, rawnorm2[x, y, z], coord[x, y, z]); CSL_BREAK;

				CSL_CASE(5u) : // XF_SRCTEX0_INROW
					coord = CSL_TERNARY((components & 32768u /* VB_HAS_UV0 */) != 0u, vec4(rawtex0[x], rawtex0[y], 1.0, 1.0), coord); CSL_BREAK;

				CSL_CASE(6u) :  // XF_SRCTEX1_INROW
					coord = CSL_TERNARY((components & 65536u /* VB_HAS_UV1 */) != 0u, vec4(rawtex1[x], rawtex1[y], 1.0, 1.0), coord); CSL_BREAK;

				CSL_CASE(7u) : // XF_SRCTEX2_INROW
					coord = CSL_TERNARY((components & 131072u /* VB_HAS_UV2 */) != 0u, vec4(rawtex2[x], rawtex2[y], 1.0, 1.0), coord); CSL_BREAK;

				CSL_CASE(8u) : // XF_SRCTEX3_INROW
					coord = CSL_TERNARY((components & 262144u /* VB_HAS_UV3 */) != 0u, vec4(rawtex3[x], rawtex3[y], 1.0, 1.0), coord); CSL_BREAK;

				CSL_CASE(9u) : // XF_SRCTEX4_INROW
					coord = CSL_TERNARY((components & 524288u /* VB_HAS_UV4 */) != 0u, vec4(rawtex4[x], rawtex4[y], 1.0, 1.0), coord); CSL_BREAK;

				CSL_CASE(10u) : // XF_SRCTEX5_INROW
					coord = CSL_TERNARY((components & 1048576u /* VB_HAS_UV5 */) != 0u, vec4(rawtex5[x], rawtex5[y], 1.0, 1.0), coord); CSL_BREAK;

				CSL_CASE(11u) : // XF_SRCTEX6_INROW
					coord = CSL_TERNARY((components & 2097152u /* VB_HAS_UV6 */) != 0u, vec4(rawtex6[x], rawtex6[y], 1.0, 1.0), coord); CSL_BREAK;

				CSL_CASE(12u) : // XF_SRCTEX7_INROW
					coord = CSL_TERNARY((components & 4194304u /* VB_HAS_UV7 */) != 0u, vec4(rawtex7[x], rawtex7[y], 1.0, 1.0), coord); CSL_BREAK;

			CSL_DEFAULT: {}
			}

			// Input form of AB11 sets z element to 1.0
			CSL_IF(bitfieldExtract(texMtxInfo, 2, 1) == 0u) // inputform == XF_TEXINPUT_AB11
				coord[z] = 1.0;

			// first transformation
			Uint texgentype = bitfieldExtract(texMtxInfo, 4, 3);
			vec3 output_tex;

			CSL_SWITCH(texgentype) {
				CSL_CASE(1u) : // XF_TEXGEN_EMBOSS_MAP
				{
					Uint light = bitfieldExtract(texMtxInfo, 15, 3);
					Uint source = bitfieldExtract(texMtxInfo, 12, 3);
					CSL_SWITCH(source) {
						CSL_CASE(0u) : { output_tex[x, y, z] = o.tex0; CSL_BREAK; }
						CSL_CASE(1u) : { output_tex[x, y, z] = o.tex1; CSL_BREAK; }
					CSL_DEFAULT: { output_tex[x, y, z] = vec3(0.0, 0.0, 0.0); CSL_BREAK; }
					}
					CSL_IF((components & 6144u) != 0u) { // VB_HAS_NRM1 | VB_HAS_NRM2
						vec3 ldir = normalize(clights[light].pos[x, y, z] - pos[x, y, z]);
						output_tex[x, y, z] += vec3(dot(ldir, _norm1), dot(ldir, _norm2), 0.0);
					}
					CSL_BREAK;
				}

				CSL_CASE(2u) : // XF_TEXGEN_COLOR_STRGBC0
				{
					output_tex[x, y, z] = vec3(o.colors_0[x], o.colors_0[y], 1.0);
					CSL_BREAK;
				}

				CSL_CASE(3u) : // XF_TEXGEN_COLOR_STRGBC1
				{
					output_tex[x, y, z] = vec3(o.colors_1[x], o.colors_1[y], 1.0);
					CSL_BREAK;
				}

			CSL_DEFAULT:  // Also XF_TEXGEN_REGULAR
				{
					CSL_IF((components & (4u /* VB_HAS_TEXMTXIDX0 */ << texgen)) != 0u) {
						// This is messy, due to dynamic indexing of the input texture coordinates.
						// Hopefully the compiler will unroll this whole loop anyway and the switch.
						Int tmp = 0;
						CSL_SWITCH(texgen) {
							CSL_CASE(0u) : { tmp = Int(rawtex0[z]); CSL_BREAK; }
							CSL_CASE(1u) : { tmp = Int(rawtex1[z]); CSL_BREAK; }
						CSL_DEFAULT: {}
						}

						CSL_IF(bitfieldExtract(texMtxInfo, 1, 1) == 1u) {
							output_tex[x, y, z] = vec3(dot(coord, ctrmtx[tmp]),
								dot(coord, ctrmtx[tmp + 1]),
								dot(coord, ctrmtx[tmp + 2]));
						} CSL_ELSE{
							output_tex[x, y, z] = vec3(dot(coord, ctrmtx[tmp]),
								dot(coord, ctrmtx[tmp + 1]),
								1.0);
						}
					} CSL_ELSE{
						CSL_IF(bitfieldExtract(texMtxInfo, 1, 1) == 1u) {
							output_tex[x, y, z] = vec3(dot(coord, ctexmtx[3u * texgen]),
								dot(coord, ctexmtx[3u * texgen + 1u]),
								dot(coord, ctexmtx[3u * texgen + 2u]));
						} CSL_ELSE {
							output_tex[x, y, z] = vec3(dot(coord, ctexmtx[3u * texgen]),
								dot(coord, ctexmtx[3u * texgen + 1u]),
								1.0);
						}
					}
					CSL_BREAK;
				}

			}

			CSL_IF(xfmem_dualTexInfo != 0u) {
				Uint postMtxInfo = (xfmem_pack1[(texgen)][y]);
				Uint base_index = bitfieldExtract(postMtxInfo, 0, 6);
				vec4 P0 = cpostmtx[base_index & 0x3fu];
				vec4 P1 = cpostmtx[(base_index + 1u) & 0x3fu];
				vec4 P2 = cpostmtx[(base_index + 2u) & 0x3fu];

				CSL_IF(bitfieldExtract(postMtxInfo, 8, 1) != 0u)
					output_tex[x, y, z] = normalize(output_tex[x, y, z]);

				// multiply by postmatrix
				output_tex[x, y, z] = vec3(dot(P0[x, y, z], output_tex[x, y, z]) + P0[w],
					dot(P1[x, y, z], output_tex[x, y, z]) + P1[w],
					dot(P2[x, y, z], output_tex[x, y, z]) + P2[w]);
			}

			CSL_IF(texgentype == 0u && output_tex[z] == 0.0) // XF_TEXGEN_REGULAR
				output_tex[x, y] = clamp(output_tex[x, y] / 2.0, vec2(-1.0, -1.0), vec2(1.0, 1.0));

			// Hopefully GPUs that can support dynamic indexing will optimize this.
			CSL_SWITCH(texgen) {
				CSL_CASE(0u) : { o.tex0 = output_tex; CSL_BREAK; }
				CSL_CASE(1u) : { o.tex1 = output_tex; CSL_BREAK; }
			CSL_DEFAULT: { }
			}
		}

		o.clipPos = o.pos;
		Float clipDepth = o.pos[z] * (1.0 - 1e-7);
		o.clipDist0 = clipDepth + o.pos[w];
		o.clipDist1 = -clipDepth;
		o.pos[z] = o.pos[w] * cpixelcenter[w] - o.pos[z] * cpixelcenter[z];
		o.pos[x, y] *= sign(cpixelcenter[x, y] * vec2(1.0, -1.0));
		o.pos[x, y] = o.pos[x, y] - o.pos[w] * cpixelcenter[x, y];
		CSL_IF(o.pos[w] == 1.0)
		{
			Float ss_pixel_x = ((o.pos[x] + 1.0) * (cviewport[x] * 0.5));
			Float ss_pixel_y = ((o.pos[y] + 1.0) * (cviewport[y] * 0.5));
			ss_pixel_x = round(ss_pixel_x);
			ss_pixel_y = round(ss_pixel_y);
			o.pos[x] = ((ss_pixel_x / (cviewport[x] * 0.5)) - 1.0);
			o.pos[y] = ((ss_pixel_y / (cviewport[y] * 0.5)) - 1.0);
		}
		vs.pos = o.pos;
		vs.colors_0 = o.colors_0;
		vs.colors_1 = o.colors_1;
		vs.tex0 = o.tex0;
		vs.tex1 = o.tex1;
		vs.clipPos = o.clipPos;
		vs.clipDist0 = o.clipDist0;
		vs.clipDist1 = o.clipDist1;
		gl_ClipDistance[0] = o.clipDist0;
		gl_ClipDistance[1] = o.clipDist1;
		gl_Position = o.pos;
	});

	return shader;
}

csl::glsl::frag_420::Shader dolphin_ubershader_fragment()
{
	using namespace csl::glsl::frag_420;
	using namespace csl::swizzles::all;
	Shader shader;

	// Pixel UberShader for 2 texgens, early-depth
	auto idot = define_function<Int, Int>("idot",
		[](ivec3 ix = "x", ivec3 iy = "y") {
		ivec3 tmp = ix * iy << "tmp";
		CSL_RETURN(tmp[x] + tmp[y] + tmp[z]);
	}, [](ivec4 ix = "x", ivec4 iy = "y") {
		ivec4 tmp = ix * iy << "tmp";
		CSL_RETURN(tmp[x] + tmp[y] + tmp[z] + tmp[w]);
	});

	auto iround = define_function<Float, vec2, vec3, vec4>("iround",
		[](Float x = "x") {
		CSL_RETURN(Int(round(x)));
	}, [](vec2 x = "x") {
		CSL_RETURN(ivec2(round(x)));
	}, [](vec3 x = "x") {
		CSL_RETURN(ivec3(round(x)));
	}, [](vec4 x = "x") {
		CSL_RETURN(ivec4(round(x)));
	});

	Qualify<sampler2DArray, Layout<Binding<0>>, Uniform, Array<8>> samp("samp");

	CSL_UNNAMED_INTERFACE_BLOCK(
		(Layout<Std140, Binding<1>>, Uniform),
		PSBlock,
		((Qualify<ivec4, Array<4>>), color),
		((Qualify<ivec4, Array<4>>), k),
		(ivec4, alphaRef),
		((Qualify<vec4, Array<8>>), texdim),
		((Qualify<ivec4, Array<2>>), czbias),
		((Qualify<ivec4, Array<2>>), cindscale),
		((Qualify<ivec4, Array<6>>), cindmtx),
		(ivec4, cfogcolor),
		(ivec4, cfogi),
		((Qualify<vec4, Array<2>>), cfogf),
		(vec4, czslope),
		(vec2, cefbscale),
		(Uint, bpmem_genmode),
		(Uint, bpmem_alphaTest),
		(Uint, bpmem_fogParam3),
		(Uint, bpmem_fogRangeBase),
		(Uint, bpmem_dstalpha),
		(Uint, bpmem_ztex_op),
		(Bool, bpmem_early_ztest),
		(Bool, bpmem_rgba6_format),
		(Bool, bpmem_dither),
		(Bool, bpmem_bounding_box),
		((Qualify<uvec4, Array<16>>), bpmem_pack1),
		((Qualify<uvec4, Array<8>>), bpmem_pack2),
		((Qualify<ivec4, Array<32>>), konstLookup)
	);

	CSL_STRUCT(VS_OUTPUT,
		(vec4, pos),
		(vec4, colors_0),
		(vec4, colors_1),
		(vec3, tex0),
		(vec3, tex1),
		(vec4, clipPos),
		(Float, clipDist0),
		(Float, clipDist1)
	);


	Qualify<vec4, Out> ocol0("ocol0"), ocol1("ocol1");


	CSL_UNNAMED_INTERFACE_BLOCK(
		In,
		VertexData,
		(vec4, pos),
		(vec4, colors_0),
		(vec4, colors_1),
		(vec3, tex0),
		(vec3, tex1),
		(vec4, clipPos),
		(Float, clipDist0),
		(Float, clipDist1)
	);

	auto selectTexCoord = define_function<vec3>("selectTexCoord",
		[&](Uint index = "index") {
		CSL_SWITCH(index) {
			CSL_CASE(0u) : {
				CSL_RETURN(tex0);
			}
			CSL_CASE(1u) : {
				CSL_RETURN(tex1);
			}
		CSL_DEFAULT: {
			CSL_RETURN(vec3(0.0, 0.0, 0.0));
			}
		}
	});

	auto sampleTexture = define_function<ivec4>("sampleTexture",
		[&](Uint sampler_num, vec2 uv) {
		CSL_RETURN(iround(texture(samp[sampler_num], vec3(uv, 0.0)) * 255.0));
	});

	auto Swizzle = define_function<ivec4>("Swizzle",
		[&](Uint s, ivec4 color) {
		// AKA: Color Channel Swapping

		ivec4 ret;
		ret[r] = color[bitfieldExtract(bpmem_pack2[(s * 2u)][y], 0, 2)];
		ret[g] = color[bitfieldExtract(bpmem_pack2[(s * 2u)][y], 2, 2)];
		ret[b] = color[bitfieldExtract(bpmem_pack2[(s * 2u + 1u)][y], 0, 2)];
		ret[a] = color[bitfieldExtract(bpmem_pack2[(s * 2u + 1u)][y], 2, 2)];
		CSL_RETURN(ret);
	});

	auto Wrap = define_function<Int>("Wrap",
		[](Int coord, Uint mode) {
		CSL_IF(mode == 0u) { // ITW_OFF
			CSL_RETURN(coord);
		} CSL_ELSE_IF(mode < 6u) { // ITW_256 to ITW_16
			CSL_RETURN(coord & (0xfffe >> Int(mode)));
		} CSL_ELSE{ // ITW_0
			CSL_RETURN(0);
		}
	});

	// TEV's Linear Interpolate, plus bias, add/subtract and scale
	auto tevLerp = define_function<Int>("tevLerp",
		[](Int A = "A", Int B = "B", Int C = "C", Int D = "D", Uint bias = "bias",
			Bool op = "op", Bool alpha = "alpha", Uint shift = "shift") {
		// Scale C from 0..255 to 0..256
		C += C >> 7;

		// Add bias to D
		CSL_IF(bias == 1u) D += 128;
		CSL_ELSE_IF(bias == 2u) D -= 128;

		Int lerp = (A << 8) + (B - A) * C;
		CSL_IF(shift != 3u) {
			lerp = lerp << Int(shift);
			D = D << Int(shift);
		}

		CSL_IF((shift == 3u) == alpha)
			lerp = lerp + CSL_TERNARY(op, 127, 128);

		Int result = lerp >> 8;

		// Add/Subtract D
		CSL_IF(op) // Subtract
			result = D - result;
		CSL_ELSE // Add
			result = D + result;

		// Most of the Shift was moved inside the lerp for improved percision
		// But we still do the divide by 2 here
		CSL_IF(shift == 3u)
			result = result >> 1;

		CSL_RETURN(result);
	});

	// TEV's Linear Interpolate, plus bias, add/subtract and scale
	auto tevLerp3 = define_function<ivec3>("tevLerp3",
		[](ivec3 A, ivec3 B, ivec3 C, ivec3 D,
			Uint bias, Bool op, Bool alpha, Uint shift) {

		// Scale C from 0..255 to 0..256
		C += C >> 7;

		// Add bias to D
		CSL_IF(bias == 1u) D += 128;
		CSL_ELSE_IF(bias == 2u) D -= 128;

		ivec3 lerp = (A << 8) + (B - A) * C;
		CSL_IF(shift != 3u) {
			lerp = lerp << Int(shift);
			D = D << Int(shift);
		}

		CSL_IF((shift == 3u) == alpha)
			lerp = lerp + CSL_TERNARY(op, 127, 128);

		ivec3 result = lerp >> 8;

		// Add/Subtract D
		CSL_IF(op) // Subtract
			result = D - result;
		CSL_ELSE // Add
			result = D + result;

		// Most of the Shift was moved inside the lerp for improved percision
		// But we still do the divide by 2 here
		CSL_IF(shift == 3u)
			result = result >> 1;

		CSL_RETURN(result);
	});

	// Implements operations 0-5 of tev's compare mode,
	// which are common to both color and alpha channels
	auto tevCompare = define_function<Bool>("tevCompare",
		[](Uint op, ivec3 color_A, ivec3 color_B) {
		CSL_SWITCH(op) {
			CSL_CASE(0u) : { // TEVCMP_R8_GT
				CSL_RETURN(color_A[r] > color_B[r]);
			}
			CSL_CASE(1u) : {// TEVCMP_R8_EQ
				CSL_RETURN(color_A[r] == color_B[r]);
			}
			CSL_CASE(2u) : { // TEVCMP_GR16_GT
				Int A_16 = (color_A[r] | (color_A[g] << 8));
				Int B_16 = (color_B[r] | (color_B[g] << 8));
				CSL_RETURN(A_16 > B_16);
			}
			CSL_CASE(3u) : { // TEVCMP_GR16_EQ
				CSL_RETURN(color_A[r] == color_B[r] && color_A[g] == color_B[g]);
			}
			CSL_CASE(4u) : { // TEVCMP_BGR24_GT
				Int A_24 = (color_A[r] | (color_A[g] << 8) | (color_A[b] << 16));
				Int B_24 = (color_B[r] | (color_B[g] << 8) | (color_B[b] << 16));
				CSL_RETURN(A_24 > B_24);
			}
			CSL_CASE(5u) : { // TEVCMP_BGR24_EQ
				CSL_RETURN(color_A[r] == color_B[r] && color_A[g] == color_B[g] && color_A[b] == color_B[b]);
			}
		CSL_DEFAULT: {
			CSL_RETURN(false);
			}
		}
	});

	// Helper function for Alpha Test
	auto alphaCompare = define_function<Bool>("alphaCompare",
		[](Int a, Int b, Uint compare) {
		CSL_SWITCH(compare) {
			CSL_CASE(0u) : {// NEVER
				CSL_RETURN(false);
			}
			CSL_CASE(1u) : {// LESS
				CSL_RETURN(a < b);
			}
			CSL_CASE(2u) : { // EQUAL
				CSL_RETURN(a == b);
			}
			CSL_CASE(3u) : {// LEQUAL
				CSL_RETURN(a <= b);
			}
			CSL_CASE(4u) : {// GREATER
				CSL_RETURN(a > b);
			}
			CSL_CASE(5u) : {// NEQUAL;
				CSL_RETURN(a != b);
			}
			CSL_CASE(6u) : {// GEQUAL
				CSL_RETURN(a >= b);
			}
			CSL_CASE(7u) : {// ALWAYS
				CSL_RETURN(true);
			}
		CSL_DEFAULT: {}
		}
	});

	CSL_STRUCT(
		State,
		((Qualify<ivec4, Array<4>>), Reg),
		(ivec4, TexColor),
		(Int, AlphaBump)
	);

	CSL_STRUCT(
		StageState,
		(Uint, stage),
		(Uint, order),
		(Uint, cc),
		(Uint, ac)
	);

	auto getRasColor = define_function<ivec4>("getRasColor",
		[&](State s, StageState ss, vec4 colors_0, vec4 colors_1) {
		// Select Ras for stage
		Uint ras = bitfieldExtract(ss.order, 7, 3);
		CSL_IF(ras < 2u) { // Lighting Channel 0 or 1
			ivec4 color = iround(CSL_TERNARY((ras == 0u), colors_0, colors_1) * 255.0);
			Uint swap = bitfieldExtract(ss.ac, 0, 2);
			CSL_RETURN(Swizzle(swap, color));
		}
		CSL_ELSE_IF(ras == 5u) { // Alpha Bumb
			CSL_RETURN(ivec4(s.AlphaBump, s.AlphaBump, s.AlphaBump, s.AlphaBump));
		}
		CSL_ELSE_IF(ras == 6u) { // Normalzied Alpha Bump
			Int normalized = s.AlphaBump | s.AlphaBump >> 5;
			CSL_RETURN(ivec4(normalized, normalized, normalized, normalized));
		}
		CSL_ELSE{
			CSL_RETURN(ivec4(0, 0, 0, 0));
		}
	});

	auto getKonstColor = define_function<ivec4>("getKonstColor",
		[&](State s, StageState ss) {
		// Select Konst for stage
		// TODO: a switch case might be better here than an dynamically  // indexed uniform lookup
		Uint tevksel = (bpmem_pack2[(ss.stage >> 1u)][y]);
		CSL_IF((ss.stage & 1u) == 0u)
			CSL_RETURN(ivec4(konstLookup[bitfieldExtract(tevksel, 4, 5)][r, g, b], konstLookup[bitfieldExtract(tevksel, 9, 5)][a]));
		CSL_ELSE
			CSL_RETURN(ivec4(konstLookup[bitfieldExtract(tevksel, 14, 5)][r, g, b], konstLookup[bitfieldExtract(tevksel, 19, 5)][a]));
	});

	auto selectColorInput = define_function<ivec3>("selectColorInput",
		[&](State s, StageState ss, vec4 colors_0,
			vec4 colors_1, Uint index) {
		CSL_SWITCH(index) {
			CSL_CASE(0u) : {// prev.rgb
				CSL_RETURN(s.Reg[0][r, g, b]);
			}
			CSL_CASE(1u) : {// prev.aaa
				CSL_RETURN(s.Reg[0][a, a, a]);
			}
			CSL_CASE(2u) : {// c0.rgb
				CSL_RETURN(s.Reg[1][r, g, b]);
			}
			CSL_CASE(3u) : {// c0.aaa
				CSL_RETURN(s.Reg[1][a, a, a]);
			}
			CSL_CASE(4u) : {// c1.rgb
				CSL_RETURN(s.Reg[2][r, g, b]);
			}
			CSL_CASE(5u) : {// c1.aaa
				CSL_RETURN(s.Reg[2][a, a, a]);
			}
			CSL_CASE(6u) : {// c2.rgb
				CSL_RETURN(s.Reg[3][r, g, b]);
			}
			CSL_CASE(7u) : {// c2.aaa
				CSL_RETURN(s.Reg[3][a, a, a]);
			}
			CSL_CASE(8u) : {
				CSL_RETURN(s.TexColor[r, g, b]);
			}
			CSL_CASE(9u) : {
				CSL_RETURN(s.TexColor[a, a, a]);
			}
			CSL_CASE(10u) : {
				CSL_RETURN(getRasColor(s, ss, colors_0, colors_1)[r, g, b]);
			}
			CSL_CASE(11u) : {
				CSL_RETURN(getRasColor(s, ss, colors_0, colors_1)[a, a, a]);
			}
			CSL_CASE(12u) : {// One
				CSL_RETURN(ivec3(255, 255, 255));
			}
			CSL_CASE(13u) : {// Half
				CSL_RETURN(ivec3(128, 128, 128));
			}
			CSL_CASE(14u) : {
				CSL_RETURN(getKonstColor(s, ss)[r, g, b]);
			}
			CSL_CASE(15u) : {// Zero
				CSL_RETURN(ivec3(0, 0, 0));
			}
		CSL_DEFAULT: {}
		}
	});

	auto selectAlphaInput = define_function<Int>("selectAlphaInput",
		[&](State s, StageState ss, vec4 colors_0, vec4 colors_1, Uint index) {
		CSL_SWITCH(index) {
			CSL_CASE(0u) : {// prev.a
				CSL_RETURN(s.Reg[0][a]);
			}
			CSL_CASE(1u) : {// c0.a
				CSL_RETURN(s.Reg[1][a]);
			}
			CSL_CASE(2u) : {// c1.a
				CSL_RETURN(s.Reg[2][a]);
			}
			CSL_CASE(3u) : {// c2.a
				CSL_RETURN(s.Reg[3][a]);
			}
			CSL_CASE(4u) : {
				CSL_RETURN(s.TexColor[a]);
			}
			CSL_CASE(5u) : {
				CSL_RETURN(getRasColor(s, ss, colors_0, colors_1)[a]);
			}
			CSL_CASE(6u) : {
				CSL_RETURN(getKonstColor(s, ss)[a]);
			}
			CSL_CASE(7u) : {// Zero
				CSL_RETURN(0);
			}
		CSL_DEFAULT: {}
		}
	});

	auto getTevReg = define_function<ivec4>("getTevReg",
		[](State s, Uint index) {
		CSL_SWITCH(index) {
			CSL_CASE(0u) : {// prev
				CSL_RETURN(s.Reg[0]);
			}
			CSL_CASE(1u) : {// c0
				CSL_RETURN(s.Reg[1]);
			}
			CSL_CASE(2u) : {// c1
				CSL_RETURN(s.Reg[2]);
			}
			CSL_CASE(3u) : {// c2
				CSL_RETURN(s.Reg[3]);
			}
		CSL_DEFAULT: {// prev
			CSL_RETURN(s.Reg[0]);
			}
		}
	});

	auto setRegColor = define_function<void>("setRegColor",
		[&](State s, Uint index, ivec3 color) {
		CSL_SWITCH(index) {
			CSL_CASE(0u) : {// prev
				s.Reg[0][r, g, b] = color;
				CSL_BREAK;
			}
			CSL_CASE(1u) : {// c0
				s.Reg[1][r, g, b] = color;
				CSL_BREAK;
			}
			CSL_CASE(2u) : {// c1
				s.Reg[2][r, g, b] = color;
				CSL_BREAK;
			}
			CSL_CASE(3u) : {// c2
				s.Reg[3][r, g, b] = color;
				CSL_BREAK;
			}
		CSL_DEFAULT: {}
		}
	});

	auto setRegAlpha = define_function<void>("setRegAlpha",
		[&](State s, Uint index, Int alpha) {
		CSL_SWITCH(index) {
			CSL_CASE(0u) : {// prev
				s.Reg[0][a] = alpha;
				CSL_BREAK;
			}
			CSL_CASE(1u) : {// c0
				s.Reg[1][a] = alpha;
				CSL_BREAK;
			}
			CSL_CASE(2u) : {// c1
				s.Reg[2][a] = alpha;
				CSL_BREAK;
			}
			CSL_CASE(3u) : {// c2
				s.Reg[3][a] = alpha;
				CSL_BREAK;
			}
		CSL_DEFAULT: {}
		}
	});

	in<Layout<Early_fragment_tests>>();

	shader.main([&] {
		vec4 rawpos = gl_FragCoord;
		ivec3 tevcoord = ivec3(0, 0, 0);
		State s;
		s.TexColor = ivec4(0, 0, 0, 0);
		s.AlphaBump = 0;

		s.Reg[0] = color[0];
		s.Reg[1] = color[1];
		s.Reg[2] = color[2];
		s.Reg[3] = color[3];
		Uint num_stages = bitfieldExtract(bpmem_genmode, 10, 4);

		// Main tev loop
		CSL_FOR(Uint stage = 0u; stage <= num_stages; stage++) {
			StageState ss;
			ss.stage = stage;
			ss.cc = (bpmem_pack1[(stage)][x, y])[x];
			ss.ac = (bpmem_pack1[(stage)][x, y])[y];
			ss.order = (bpmem_pack2[(stage >> 1u)][x]);
			CSL_IF((stage & 1u) == 1u)
				ss.order = ss.order >> 12u;

			Uint tex_coord = bitfieldExtract(ss.order, 3, 3);
			vec3 uv = selectTexCoord((tex_coord));
			ivec2 fixedPoint_uv = ivec2(CSL_TERNARY(uv[z] == 0.0, uv[x, y], (uv[x, y] / uv[z])) * texdim[tex_coord][z, w]);

			Bool texture_enabled = (ss.order & 64u) != 0u;

			// Indirect textures
			Uint tevind = (bpmem_pack1[(stage)][z]);
			CSL_IF(tevind != 0u) {
				Uint bs = bitfieldExtract(tevind, 7, 2);
				Uint fmt = bitfieldExtract(tevind, 2, 2);
				Uint bias = bitfieldExtract(tevind, 4, 3);
				Uint bt = bitfieldExtract(tevind, 0, 2);
				Uint mid = bitfieldExtract(tevind, 9, 4);

				ivec3 indcoord;
				{
					Uint iref = (bpmem_pack1[(bt)][w]);
					CSL_IF(iref != 0u)
					{
						Uint texcoord = bitfieldExtract(iref, 0, 3);
						Uint texmap = bitfieldExtract(iref, 8, 3);
						vec3 uv = selectTexCoord((texcoord));
						ivec2 fixedPoint_uv = ivec2(CSL_TERNARY(uv[z] == 0.0, uv[x, y], (uv[x, y] / uv[z])) * texdim[texcoord][z, w]);

						CSL_IF((bt & 1u) == 0u)
							fixedPoint_uv = fixedPoint_uv >> cindscale[bt >> 1u][x, y];
						CSL_ELSE
							fixedPoint_uv = fixedPoint_uv >> cindscale[bt >> 1u][z, w];

						indcoord = sampleTexture(texmap, vec2(fixedPoint_uv) * texdim[texmap][x, y])[a, b, g];
					} CSL_ELSE{
						indcoord = ivec3(0, 0, 0);
					}
				}
				CSL_IF(bs != 0u)
					s.AlphaBump = indcoord[bs - 1u];

				CSL_SWITCH(fmt) {
					CSL_CASE(0u) : {
						indcoord[x] = indcoord[x] + CSL_TERNARY((bias & 1u) != 0u, -128, 0);
						indcoord[y] = indcoord[y] + CSL_TERNARY((bias & 2u) != 0u, -128, 0);
						indcoord[z] = indcoord[z] + CSL_TERNARY((bias & 4u) != 0u, -128, 0);
						s.AlphaBump = s.AlphaBump & 0xf8;
						CSL_BREAK;
					}
					CSL_CASE(1u) : {
						indcoord[x] = (indcoord[x] & 0x1f) + CSL_TERNARY((bias & 1u) != 0u, 1, 0);
						indcoord[y] = (indcoord[y] & 0x1f) + CSL_TERNARY((bias & 2u) != 0u, 1, 0);
						indcoord[z] = (indcoord[z] & 0x1f) + CSL_TERNARY((bias & 4u) != 0u, 1, 0);
						s.AlphaBump = s.AlphaBump & 0xe0;
						CSL_BREAK;
					}
					CSL_CASE(2u) : {
						indcoord[x] = (indcoord[x] & 0x0f) + CSL_TERNARY((bias & 1u) != 0u, 1, 0);
						indcoord[y] = (indcoord[y] & 0x0f) + CSL_TERNARY((bias & 2u) != 0u, 1, 0);
						indcoord[z] = (indcoord[z] & 0x0f) + CSL_TERNARY((bias & 4u) != 0u, 1, 0);
						s.AlphaBump = s.AlphaBump & 0xf0;
						CSL_BREAK;
					}
					CSL_CASE(3u) : {
						indcoord[x] = (indcoord[x] & 0x07) + CSL_TERNARY((bias & 1u) != 0u, 1, 0);
						indcoord[y] = (indcoord[y] & 0x07) + CSL_TERNARY((bias & 2u) != 0u, 1, 0);
						indcoord[z] = (indcoord[z] & 0x07) + CSL_TERNARY((bias & 4u) != 0u, 1, 0);
						s.AlphaBump = s.AlphaBump & 0xf8;
						CSL_BREAK;
					}
				CSL_DEFAULT: {}
				}

				// Matrix multiply
				ivec2 indtevtrans = ivec2(0, 0);
				CSL_IF((mid & 3u) != 0u) {
					Uint mtxidx = 2u * ((mid & 3u) - 1u);
					Int shift = cindmtx[mtxidx][w];

					CSL_SWITCH(mid >> 2u) {
						CSL_CASE(0u) : {// 3x2 S0.10 matrix
							indtevtrans = ivec2(idot(cindmtx[mtxidx][x, y, z], indcoord), idot(cindmtx[mtxidx + 1u][x, y, z], indcoord)) >> 3;
							CSL_BREAK;
						}
						CSL_CASE(1u) : {// S matrix, S17.7 format
							indtevtrans = (fixedPoint_uv * indcoord[x, x]) >> 8;
							CSL_BREAK;
						}
						CSL_CASE(2u) : {// T matrix, S17.7 format
							indtevtrans = (fixedPoint_uv * indcoord[y, y]) >> 8;
							CSL_BREAK;
						}
					CSL_DEFAULT: {}
					}

					CSL_IF(shift >= 0)
						indtevtrans = indtevtrans >> shift;
					CSL_ELSE
						indtevtrans = indtevtrans << ((-shift) & 31);

				}

				// Wrapping
				Uint sw = bitfieldExtract(tevind, 13, 3);
				Uint tw = bitfieldExtract(tevind, 16, 3);
				ivec2 wrapped_coord = ivec2(Wrap(fixedPoint_uv[x], sw), Wrap(fixedPoint_uv[y], tw));

				CSL_IF((tevind & 1048576u) != 0u) // add previous tevcoord
					tevcoord[x, y] += wrapped_coord + indtevtrans;
				CSL_ELSE
					tevcoord[x, y] = wrapped_coord + indtevtrans;

				// Emulate s24 overflows
				tevcoord[x, y] = (tevcoord[x, y] << 8) >> 8;

			} CSL_ELSE_IF(texture_enabled) {
				tevcoord[x, y] = fixedPoint_uv;
			}

			// Sample texture for stage
			CSL_IF(texture_enabled) {
				Uint sampler_num = bitfieldExtract(ss.order, 0, 3);

				vec2 uv = (vec2(tevcoord[x, y])) * texdim[sampler_num][x, y];

				ivec4 color = sampleTexture(sampler_num, uv);

				Uint swap = bitfieldExtract(ss.ac, 2, 2);
				s.TexColor = Swizzle(swap, color);
			} CSL_ELSE{
				// Texture is OpFlags::DISABLED
				s.TexColor = ivec4(255, 255, 255, 255);
			}

				// This is the Meat of TEV
			{
				// Color Combiner
				Uint color_a = bitfieldExtract(ss.cc, 12, 4);
				Uint color_b = bitfieldExtract(ss.cc, 8, 4);
				Uint color_c = bitfieldExtract(ss.cc, 4, 4);
				Uint color_d = bitfieldExtract(ss.cc, 0, 4);
				Uint color_bias = bitfieldExtract(ss.cc, 16, 2);
				Bool color_op = Bool(bitfieldExtract(ss.cc, 18, 1));
				Bool color_clamp = Bool(bitfieldExtract(ss.cc, 19, 1));
				Uint color_shift = bitfieldExtract(ss.cc, 20, 2);
				Uint color_dest = bitfieldExtract(ss.cc, 22, 2);
				Uint color_compare_op = color_shift << 1u | Uint(color_op);

				ivec3 color_A = selectColorInput(s, ss, colors_0, colors_1, color_a) & ivec3(255, 255, 255);
				ivec3 color_B = selectColorInput(s, ss, colors_0, colors_1, color_b) & ivec3(255, 255, 255);
				ivec3 color_C = selectColorInput(s, ss, colors_0, colors_1, color_c) & ivec3(255, 255, 255);
				ivec3 color_D = selectColorInput(s, ss, colors_0, colors_1, color_d);  // 10 bits + sign

				ivec3 color;
				CSL_IF(color_bias != 3u) { // Normal mode
					color = tevLerp3(color_A, color_B, color_C, color_D, color_bias, color_op, false, color_shift);
				} CSL_ELSE{ // Compare mode
					// op 6 and 7 do a select per color channel
					CSL_IF(color_compare_op == 6u) {
					// TEVCMP_RGB8_GT
					color[r] = CSL_TERNARY((color_A[r] > color_B[r]), color_C[r], 0);
					color[g] = CSL_TERNARY((color_A[g] > color_B[g]), color_C[g], 0);
					color[b] = CSL_TERNARY((color_A[b] > color_B[b]), color_C[b], 0);
				} CSL_ELSE_IF(color_compare_op == 7u) {
					// TEVCMP_RGB8_EQ
					color[r] = CSL_TERNARY((color_A[r] == color_B[r]), color_C[r], 0);
					color[g] = CSL_TERNARY((color_A[g] == color_B[g]), color_C[g], 0);
					color[b] = CSL_TERNARY((color_A[b] > color_B[b]), color_C[b], 0);
				} CSL_ELSE {
					// The remaining ops do one compare which selects all 3 channels
					color = CSL_TERNARY(tevCompare(color_compare_op, color_A, color_B), color_C, ivec3(0, 0, 0));
				}
				color = color_D + color;
				}

					// Clamp result
					CSL_IF(color_clamp)
					color = clamp(color, 0, 255);
				CSL_ELSE
					color = clamp(color, -1024, 1023);

				// Write result to the correct input register of the next stage
				setRegColor(s, color_dest, color);

				// Alpha Combiner
				Uint alpha_a = bitfieldExtract(ss.ac, 13, 3);
				Uint alpha_b = bitfieldExtract(ss.ac, 10, 3);
				Uint alpha_c = bitfieldExtract(ss.ac, 7, 3);
				Uint alpha_d = bitfieldExtract(ss.ac, 4, 3);
				Uint alpha_bias = bitfieldExtract(ss.ac, 16, 2);
				Bool alpha_op = Bool(bitfieldExtract(ss.ac, 18, 1));
				Bool alpha_clamp = Bool(bitfieldExtract(ss.ac, 19, 1));
				Uint alpha_shift = bitfieldExtract(ss.ac, 20, 2);
				Uint alpha_dest = bitfieldExtract(ss.ac, 22, 2);
				Uint alpha_compare_op = alpha_shift << 1u | Uint(alpha_op);

				Int alpha_A;
				Int alpha_B;
				CSL_IF(alpha_bias != 3u || alpha_compare_op > 5u) {
					// Small optimisation here: alpha_A and alpha_B are unused by compare ops 0-5
					alpha_A = selectAlphaInput(s, ss, colors_0, colors_1, alpha_a) & 255;
					alpha_B = selectAlphaInput(s, ss, colors_0, colors_1, alpha_b) & 255;
				}
				Int alpha_C = selectAlphaInput(s, ss, colors_0, colors_1, alpha_c) & 255;
				Int alpha_D = selectAlphaInput(s, ss, colors_0, colors_1, alpha_d); // 10 bits + sign

				Int alpha;
				CSL_IF(alpha_bias != 3u) { // Normal mode
					alpha = tevLerp(alpha_A, alpha_B, alpha_C, alpha_D, alpha_bias, alpha_op, true, alpha_shift);
				} CSL_ELSE{ // Compare mode
					CSL_IF(alpha_compare_op == 6u) {
					// TEVCMP_A8_GT
					alpha = CSL_TERNARY((alpha_A > alpha_B), alpha_C, 0);
				} CSL_ELSE_IF(alpha_compare_op == 7u) {
					// TEVCMP_A8_EQ
					alpha = CSL_TERNARY((alpha_A == alpha_B), alpha_C, 0);
				} CSL_ELSE {
					// All remaining alpha compare ops actually compare the color channels
					alpha = CSL_TERNARY(tevCompare(alpha_compare_op, color_A, color_B), alpha_C, 0);
				}
				alpha = alpha_D + alpha;
				}

					// Clamp result
					CSL_IF(alpha_clamp)
					alpha = clamp(alpha, 0, 255);
				CSL_ELSE
					alpha = clamp(alpha, -1024, 1023);

				// Write result to the correct input register of the next stage
				setRegAlpha(s, alpha_dest, alpha);
			}
		} // Main tev loop

		ivec4 TevResult;
		TevResult[x, y, z] = getTevReg(s, bitfieldExtract((bpmem_pack1[(num_stages)][x, y])[x], 22, 2))[x, y, z];
		TevResult[w] = getTevReg(s, bitfieldExtract((bpmem_pack1[(num_stages)][x, y])[y], 22, 2))[w];
		TevResult &= 255;

		Int zCoord = Int(rawpos[z] * 16777216.0);
		zCoord = clamp(zCoord, 0, 0xFFFFFF);

		// Depth Texture
		Int early_zCoord = zCoord;
		CSL_IF(bpmem_ztex_op != 0u) {
			Int ztex = Int(czbias[1][w]); // fixed bias

			// Whatever texture was in our last stage, it's now our depth texture
			ztex += idot(s.TexColor[x, y, z, w], czbias[0][x, y, z, w]);
			ztex += CSL_TERNARY((bpmem_ztex_op == 1u), zCoord, 0);
			zCoord = ztex & 0xFFFFFF;
		}

		// Alpha Test
		CSL_IF(bpmem_alphaTest != 0u) {
			Bool comp0 = alphaCompare(TevResult[a], alphaRef[r], bitfieldExtract(bpmem_alphaTest, 16, 3));
			Bool comp1 = alphaCompare(TevResult[a], alphaRef[g], bitfieldExtract(bpmem_alphaTest, 19, 3));

			// These if statements are written weirdly to work around intel and qualcom bugs with handling booleans.
			CSL_SWITCH(bitfieldExtract(bpmem_alphaTest, 22, 2)) {
				CSL_CASE(0u) : {// AND
					CSL_IF(comp0&& comp1) { CSL_BREAK; } CSL_ELSE{ CSL_DISCARD; CSL_BREAK };
				}
				CSL_CASE(1u) : {// OR
					CSL_IF(comp0 || comp1) { CSL_BREAK; } CSL_ELSE{ CSL_DISCARD; CSL_BREAK };
				}
				CSL_CASE(2u) : {// XOR
					CSL_IF(comp0 != comp1) { CSL_BREAK; } CSL_ELSE{ CSL_DISCARD; CSL_BREAK };
				}
				CSL_CASE(3u) : {// XNOR
					CSL_IF(comp0 == comp1) { CSL_BREAK; } CSL_ELSE{ CSL_DISCARD; CSL_BREAK };
				}
			CSL_DEFAULT: {}
			}
		}

		CSL_IF(bpmem_dither) {
			// Flipper uses a standard 2x2 Bayer Matrix for 6 bit dithering
			// Here the matrix is encoded into the two factor constants
			ivec2 dither = ivec2(rawpos[x, y]) & 1;
			TevResult[r, g, b] = (TevResult[r, g, b] - (TevResult[r, g, b] >> 6)) + abs(dither[y] * 3 - dither[x] * 2);
		}

		// Fog
		Uint fog_function = bitfieldExtract(bpmem_fogParam3, 21, 3);
		CSL_IF(fog_function != 0u) {
			// TODO: This all needs to be converted from float to fixed point
			Float ze;
			CSL_IF(bitfieldExtract(bpmem_fogParam3, 20, 1) == 0u) {
				// perspective
				// ze = A/(B - (Zs >> B_SHF)
				ze = (cfogf[1][x] * 16777216.0) / Float(cfogi[y] - (zCoord >> cfogi[w]));
			} CSL_ELSE{
				// orthographic
				// ze = a*Zs    (here, no B_SHF)
				ze = cfogf[1][x] * Float(zCoord) / 16777216.0;
			}

				CSL_IF(Bool(bitfieldExtract(bpmem_fogRangeBase, 10, 1))) {
				// x_adjust = sqrt((x-center)^2 + k^2)/k
				// ze *= x_adjust
				// TODO Instead of this theoretical calculation, we should use the
				//      coefficient table given in the fog range BP registers!
				Float x_adjust = (2.0 * (rawpos[x] / cfogf[0][y])) - 1.0 - cfogf[0][x];
				x_adjust = sqrt(x_adjust * x_adjust + cfogf[0][z] * cfogf[0][z]) / cfogf[0][z];
				ze *= x_adjust;
			}

			Float fog = clamp(ze - cfogf[1][z], 0.0, 1.0);

			CSL_IF(fog_function > 3u) {
				CSL_SWITCH(fog_function) {
					CSL_CASE(4u) : {
						fog = 1.0 - exp2(-8.0 * fog);
						CSL_BREAK;;
					}
					CSL_CASE(5u) : {
						fog = 1.0 - exp2(-8.0 * fog * fog);
						CSL_BREAK;
					}
					CSL_CASE(6u) : {
						fog = exp2(-8.0 * (1.0 - fog));
						CSL_BREAK;
					}
					CSL_CASE(7u) : {
						fog = 1.0 - fog;
						fog = exp2(-8.0 * fog * fog);
						CSL_BREAK;
					}
				CSL_DEFAULT: {}
				}
			}

			Int ifog = iround(fog * 256.0);
			TevResult[r, g, b] = (TevResult[r, g, b] * (256 - ifog) + cfogcolor[r, g, b] * ifog) >> 8;

			CSL_IF(bpmem_rgba6_format)
				ocol0[r, g, b] = vec3(TevResult[r, g, b] >> 2) / 63.0;
			CSL_ELSE
				ocol0[r, g, b] = vec3(TevResult[r, g, b]) / 255.0;

			CSL_IF(bpmem_dstalpha != 0u)
				ocol0[a] = Float(bitfieldExtract(bpmem_dstalpha, 0, 8) >> 2u) / 63.0;
			CSL_ELSE
				ocol0[a] = Float(TevResult[a] >> 2) / 63.0;

			// Dest alpha override (dual source blending)
			// Colors will be blended against the alpha from ocol1 and
			// the alpha from ocol0 will be written to the framebuffer.
			ocol1 = vec4(0.0, 0.0, 0.0, Float(TevResult[a]) / 255.0);
		}
	});

	return shader;
}