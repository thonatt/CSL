#include "dolphin.h"

#include <csl/Shaders.h>

std::string dolphinVertex() {

	using namespace csl::vert_430;
	using namespace csl::swizzles::all;

	Shader shader;

	GL_STRUCT(Light,
		(ivec4) color,
		(vec4) cosatt,
		(vec4) distatt,
		(vec4) pos,
		(vec4) dir
	);

	using VSBlockQualifier = Uniform<Layout<Binding<2>, Std140>>;
	GL_INTERFACE_BLOCK(VSBlockQualifier, VSBLock, , ,
		(Uint) components,
		(Uint) xfmem_dualTexInfo,
		(Uint) xfmem_numColorChans,
		(GetArray<vec4>::Size<6>) cpnmtx,
		(GetArray<vec4>::Size<4>) cproj,
		(GetArray<ivec4>::Size<6>) cmtrl,
		(GetArray<Light>::Size<8>) clights,
		(GetArray<vec4>::Size<24>) ctexmtx,
		(GetArray<vec4>::Size<64>) ctrmtx,
		(GetArray<vec4>::Size<32>) cnmtx,
		(GetArray<vec4>::Size<64>) cpostmtx,
		(vec4) cpixelcenter,
		(vec2) cviewport,
		(GetArray<uvec4>::Size<8>) xfmem_pack1
	);

	GL_STRUCT(VS_OUTPUT,
		(vec4) pos,
		(vec4) colors_0,
		(vec4) colors_1,
		(vec3) tex0,
		(vec3) tex1,
		(vec4) clipPos,
		(Float) clipDist0,
		(Float) clipDist1
	);

	auto CalculateLighting = declareFunc<ivec4>("CalculateLighting",
		[&](Uint index = "index", Uint attnfunc = "attnfunc", Uint diffusefunc = "diffusefunc",
			vec3 pos = "pos", vec3 normal = "normal") {
		vec3 ldir("ldir"), h("h"), cosAttn("cosAttn"), distAttn("distAttn");
		Float dist("dist"), dist2("dist2"), attn("attn");

		GL_SWITCH(attnfunc) {
			// LIGNTATTN_NONE
			GL_CASE(0u) : { }

			// LIGHTATTN_DIR
			GL_CASE(2u) : {
				ldir = normalize(clights[index].pos[x, y, z] - pos[x, y, z]);
				attn = 1.0;
				GL_IF(length(ldir) == 0.0)
					ldir = normal;
				GL_BREAK;
			}

			// LIGHTATTN_SPEC
			GL_CASE(1u) : {
				ldir = normalize(clights[index].pos[x, y, z] - pos[x, y, z]);
				attn = GL_TERNARY(dot(normal, ldir) >= 0.0, max(0.0, dot(normal, clights[index].dir[x, y, z])), 0.0);
				cosAttn = clights[index].cosatt[x, y, z];
				GL_IF(diffusefunc == 0u) // LIGHTDIF_NONE
					distAttn = clights[index].distatt[x, y, z];
				GL_ELSE
					distAttn = normalize(clights[index].distatt[x, y, z]);
				attn = max(0.0, dot(cosAttn, vec3(1.0, attn, attn*attn))) / dot(distAttn, vec3(1.0, attn, attn*attn));
				GL_BREAK;
			}

			// LIGHTATTN_SPOT
			GL_CASE(3u) : {
				ldir = clights[index].pos[x, y, z] - pos[x, y, z];
				dist2 = dot(ldir, ldir);
				dist = sqrt(dist2);
				ldir = ldir / dist;
				attn = max(0.0, dot(ldir, clights[index].dir[x, y, z]));
				attn = max(0.0, clights[index].cosatt[x] + clights[index].cosatt[y] * attn + clights[index].cosatt[z] * attn * attn) / dot(clights[index].distatt[x, y, z], vec3(1.0, dist, dist2));
				GL_BREAK;
			}

		GL_DEFAULT: {
			attn = 1.0;
			ldir = normal;
			GL_BREAK;
			}
		}

		GL_SWITCH(diffusefunc) {
			// LIGHTDIF_NONE
			GL_CASE(0u) : {
				GL_RETURN(ivec4(round(attn * vec4(clights[index].color))));
			}

			// LIGHTDIF_SIGN
			GL_CASE(1u) : {
				GL_RETURN(ivec4(round(attn * dot(ldir, normal) * vec4(clights[index].color))));
			}

			GL_CASE(2u) : {
				GL_RETURN(ivec4(round(attn * max(0.0, dot(ldir, normal)) * vec4(clights[index].color))));
			}

		GL_DEFAULT: {
			GL_RETURN(ivec4(0, 0, 0, 0));
			}
		}
	});

	In<vec4> rawpos("rawpos");
	In<uvec4> posmtx("posmtx");
	In<vec3> rawnorm0("rawnorm0"), rawnorm1("rawnorm1"), rawnorm2("rawnorm2");
	In<vec4> rawcolor0("rawcolor0"), rawcolor1("rawcolor1");
	In<vec3> rawtex0("rawtex0"), rawtex1("rawtex1"), rawtex2("rawtex2"), rawtex3("rawtex3"), rawtex4("rawtex4"), rawtex5("rawtex5"), rawtex6("rawtex6"), rawtex7("rawtex7");

	GL_INTERFACE_BLOCK(Out<>, VertexData, vs, ,
		(vec4) pos,
		(vec4) colors_0,
		(vec4) colors_1,
		(vec3) tex0,
		(vec3) tex1,
		(vec4) clipPos,
		(Float) clipDist0,
		(Float) clipDist1
	);

	shader.main([&] {
		VS_OUTPUT o("o");

		// Position matrix
		vec4 P0, P1, P2;

		// Normal matrix
		vec3 N0, N1, N2;

		GL_IF((components & 2u) != 0u) {// VB_HAS_POSMTXIDX
			// Vertex format has a per-vertex matrix
			Int posidx = Int(posmtx[r]);
			P0 = ctrmtx[posidx];
			P1 = ctrmtx[posidx + 1];
			P2 = ctrmtx[posidx + 2];

			Int normidx = GL_TERNARY(posidx >= 32, (posidx - 32), posidx);
			N0 = cnmtx[normidx][x, y, z];
			N1 = cnmtx[normidx + 1][x, y, z];
			N2 = cnmtx[normidx + 2][x, y, z];
		} GL_ELSE{
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
		GL_IF((components & 1024u) != 0u) // VB_HAS_NRM0
			_norm0 = normalize(vec3(dot(N0, rawnorm0), dot(N1, rawnorm0), dot(N2, rawnorm0)));

		vec3 _norm1 = vec3(0.0, 0.0, 0.0);
		GL_IF((components & 2048u) != 0u) // VB_HAS_NRM1
			_norm1 = vec3(dot(N0, rawnorm1), dot(N1, rawnorm1), dot(N2, rawnorm1));

		vec3 _norm2 = vec3(0.0, 0.0, 0.0);
		GL_IF((components & 4096u) != 0u) // VB_HAS_NRM2
			_norm2 = vec3(dot(N0, rawnorm2), dot(N1, rawnorm2), dot(N2, rawnorm2));

		// Lighting
		GL_FOR(Uint chan = Uint(0u) << "chan"; chan < xfmem_numColorChans; chan++) {
			Uint colorreg = (xfmem_pack1[(chan)][z]);
			Uint alphareg = (xfmem_pack1[(chan)][w]);
			ivec4 mat = cmtrl[chan + 2u];
			ivec4 lacc = ivec4(255, 255, 255, 255);

			GL_IF(bitfieldExtract(colorreg, 0, 1) != 0u) {
				GL_IF((components & (8192u << chan)) != 0u) // VB_HAS_COL0
					mat[x, y, z] = ivec3(round(GL_TERNARY(chan == 0u, rawcolor0[x, y, z], rawcolor1[x, y, z]) * 255.0));
				GL_ELSE_IF((components & 8192u) != 0u) // VB_HAS_COLO0
					mat[x, y, z] = ivec3(round(rawcolor0[x, y, z] * 255.0));
				GL_ELSE
					mat[x, y, z] = ivec3(255, 255, 255);
			}

			GL_IF(bitfieldExtract(alphareg, 0, 1) != 0u) {
				GL_IF((components & (8192u << chan)) != 0u) // VB_HAS_COL0
					mat[w] = Int(round(GL_TERNARY(chan == 0u, rawcolor0[w], rawcolor1[w]) * 255.0));
				GL_ELSE_IF((components & 8192u) != 0u) // VB_HAS_COLO0
					mat[w] = Int(round(rawcolor0[w] * 255.0));
				GL_ELSE
					mat[w] = 255;
			} GL_ELSE{
				mat[w] = cmtrl[chan + 2u][w];
			}

				GL_IF(bitfieldExtract(colorreg, 1, 1) != 0u) {
				GL_IF(bitfieldExtract(colorreg, 6, 1) != 0u) {
					GL_IF((components & (8192u << chan)) != 0u) // VB_HAS_COL0
						lacc[x, y, z] = ivec3(round(GL_TERNARY(chan == 0u, rawcolor0[x, y, z], rawcolor1[x, y, z]) * 255.0));
					GL_ELSE_IF((components & 8192u) != 0u) // VB_HAS_COLO0
						lacc[x, y, z] = ivec3(round(rawcolor0[x, y, z] * 255.0));
					GL_ELSE
						lacc[x, y, z] = ivec3(255, 255, 255);
				} GL_ELSE{
					lacc[x, y, z] = cmtrl[chan][x, y, z];
				}

				Uint light_mask = bitfieldExtract(colorreg, 2, 4) | (bitfieldExtract(colorreg, 11, 4) << 4u);
				Uint attnfunc = bitfieldExtract(colorreg, 9, 2);
				Uint diffusefunc = bitfieldExtract(colorreg, 7, 2);
				GL_FOR(Uint light_index = 0u; light_index < 8u; light_index++) {
					GL_IF((light_mask & (1u << light_index)) != 0u)
						lacc[x, y, z] += CalculateLighting(light_index, attnfunc, diffusefunc, pos[x, y, z], _norm0)[x, y, z];
				}
			}

			GL_IF(bitfieldExtract(alphareg, 1, 1) != 0u) {
				GL_IF(bitfieldExtract(alphareg, 6, 1) != 0u) {
					GL_IF((components & (8192u << chan)) != 0u) // VB_HAS_COL0
						lacc[w] = Int(round(GL_TERNARY(chan == 0u, rawcolor0[w], rawcolor1[w]) * 255.0));
					GL_ELSE_IF((components & 8192u) != 0u) // VB_HAS_COLO0
						lacc[w] = Int(round(rawcolor0[w] * 255.0));
					GL_ELSE
						lacc[w] = 255;
				} GL_ELSE{
					lacc[w] = cmtrl[chan][w];
				}

				Uint light_mask = bitfieldExtract(alphareg, 2, 4) | (bitfieldExtract(alphareg, 11, 4) << 4u);
				Uint attnfunc = bitfieldExtract(alphareg, 9, 2);
				Uint diffusefunc = bitfieldExtract(alphareg, 7, 2);
				GL_FOR(Uint light_index = 0u; light_index < 8u; light_index++) {
					GL_IF((light_mask & (1u << light_index)) != 0u)
						lacc[w] += CalculateLighting(light_index, attnfunc, diffusefunc, pos[x, y, z], _norm0)[w];
				}
			}

			lacc = clamp(lacc, 0, 255);

			// Hopefully GPUs that can support dynamic indexing will optimize this.
			vec4 lit_color = vec4((mat * (lacc + (lacc >> 7))) >> 8) / 255.0;
			GL_SWITCH(chan) {
				GL_CASE(0u) : { o.colors_0 = lit_color; GL_BREAK; }
				GL_CASE(1u) : { o.colors_1 = lit_color; GL_BREAK; }
			GL_DEFAULT: { }
			}
		}

		GL_IF(xfmem_numColorChans < 2u && (components & 16384u) == 0u)
			o.colors_1 = o.colors_0;

		o.tex0 = vec3(0.0, 0.0, 0.0);
		o.tex1 = vec3(0.0, 0.0, 0.0);

		GL_FOR(Uint texgen = 0u; texgen < 2u; texgen++) {
			// Texcoord transforms
			vec4 coord = vec4(0.0, 0.0, 1.0, 1.0);
			Uint texMtxInfo = (xfmem_pack1[(texgen)][x]);

			GL_SWITCH(bitfieldExtract(texMtxInfo, 7, 5)) {
				GL_CASE(0u) : // XF_SRCGEOM_INROW
					coord[x, y, z] = rawpos[x, y, z]; GL_BREAK;

				GL_CASE(1u) : // XF_SRCNORMAL_INROW
					coord[x, y, z] = GL_TERNARY((components & 1024u /* VB_HAS_NRM0 */) != 0u, rawnorm0[x, y, z], coord[x, y, z]); GL_BREAK;

				GL_CASE(3u) : // XF_SRCBINORMAL_T_INROW
					coord[x, y, z] = GL_TERNARY((components & 2048u /* VB_HAS_NRM1 */) != 0u, rawnorm1[x, y, z], coord[x, y, z]); GL_BREAK;

				GL_CASE(4u) : // XF_SRCBINORMAL_B_INROW
					coord[x, y, z] = GL_TERNARY((components & 4096u /* VB_HAS_NRM2 */) != 0u, rawnorm2[x, y, z], coord[x, y, z]); GL_BREAK;

				GL_CASE(5u) : // XF_SRCTEX0_INROW
					coord[x, y, z] = GL_TERNARY((components & 32768u /* VB_HAS_UV0 */) != 0u, vec4(rawtex0[x], rawtex0[y], 1.0, 1.0), coord); GL_BREAK;

				GL_CASE(6u) :  // XF_SRCTEX1_INROW
					coord[x, y, z] = GL_TERNARY((components & 65536u /* VB_HAS_UV1 */) != 0u, vec4(rawtex1[x], rawtex1[y], 1.0, 1.0), coord); GL_BREAK;

				GL_CASE(7u) : // XF_SRCTEX2_INROW
					coord[x, y, z] = GL_TERNARY((components & 131072u /* VB_HAS_UV2 */) != 0u, vec4(rawtex2[x], rawtex2[y], 1.0, 1.0), coord); GL_BREAK;

				GL_CASE(8u) : // XF_SRCTEX3_INROW
					coord[x, y, z] = GL_TERNARY((components & 262144u /* VB_HAS_UV3 */) != 0u, vec4(rawtex3[x], rawtex3[y], 1.0, 1.0), coord); GL_BREAK;

				GL_CASE(9u) : // XF_SRCTEX4_INROW
					coord[x, y, z] = GL_TERNARY((components & 524288u /* VB_HAS_UV4 */) != 0u, vec4(rawtex4[x], rawtex4[y], 1.0, 1.0), coord); GL_BREAK;

				GL_CASE(10u) : // XF_SRCTEX5_INROW
					coord[x, y, z] = GL_TERNARY((components & 1048576u /* VB_HAS_UV5 */) != 0u, vec4(rawtex5[x], rawtex5[y], 1.0, 1.0), coord); GL_BREAK;

				GL_CASE(11u) : // XF_SRCTEX6_INROW
					coord[x, y, z] = GL_TERNARY((components & 2097152u /* VB_HAS_UV6 */) != 0u, vec4(rawtex6[x], rawtex6[y], 1.0, 1.0), coord); GL_BREAK;

				GL_CASE(12u) : // XF_SRCTEX7_INROW
					coord[x, y, z] = GL_TERNARY((components & 4194304u /* VB_HAS_UV7 */) != 0u, vec4(rawtex7[x], rawtex7[y], 1.0, 1.0), coord); GL_BREAK;

			GL_DEFAULT: {}
			}

			// Input form of AB11 sets z element to 1.0
			GL_IF(bitfieldExtract(texMtxInfo, 2, 1) == 0u) // inputform == XF_TEXINPUT_AB11
				coord[z] = 1.0;

			// first transformation
			Uint texgentype = bitfieldExtract(texMtxInfo, 4, 3);
			vec3 output_tex;

			GL_SWITCH(texgentype) {
				GL_CASE(1u) : // XF_TEXGEN_EMBOSS_MAP
				{
					Uint light = bitfieldExtract(texMtxInfo, 15, 3);
					Uint source = bitfieldExtract(texMtxInfo, 12, 3);
					GL_SWITCH(source) {
						GL_CASE(0u) : { output_tex[x, y, z] = o.tex0; GL_BREAK; }
						GL_CASE(1u) : { output_tex[x, y, z] = o.tex1; GL_BREAK; }
					GL_DEFAULT: { output_tex[x, y, z] = vec3(0.0, 0.0, 0.0); GL_BREAK; }
					}
					GL_IF((components & 6144u) != 0u) { // VB_HAS_NRM1 | VB_HAS_NRM2
						vec3 ldir = normalize(clights[light].pos[x, y, z] - pos[x, y, z]);
						output_tex[x, y, z] += vec3(dot(ldir, _norm1), dot(ldir, _norm2), 0.0);
					}
					GL_BREAK;
				}

				GL_CASE(2u) : // XF_TEXGEN_COLOR_STRGBC0
				{
					output_tex[x, y, z] = vec3(o.colors_0[x], o.colors_0[y], 1.0);
					GL_BREAK;
				}

				GL_CASE(3u) : // XF_TEXGEN_COLOR_STRGBC1
				{
					output_tex[x, y, z] = vec3(o.colors_1[x], o.colors_1[y], 1.0);
					GL_BREAK;
				}

			GL_DEFAULT:  // Also XF_TEXGEN_REGULAR
				{
					GL_IF((components & (4u /* VB_HAS_TEXMTXIDX0 */ << texgen)) != 0u) {
						// This is messy, due to dynamic indexing of the input texture coordinates.
						// Hopefully the compiler will unroll this whole loop anyway and the switch.
						Int tmp = 0;
						GL_SWITCH(texgen) {
							GL_CASE(0u) : { tmp = Int(rawtex0[z]); GL_BREAK; }
							GL_CASE(1u) : { tmp = Int(rawtex1[z]); GL_BREAK; }
						GL_DEFAULT: {}
						}

						GL_IF(bitfieldExtract(texMtxInfo, 1, 1) == 1u) {
							output_tex[x, y, z] = vec3(dot(coord, ctrmtx[tmp]),
								dot(coord, ctrmtx[tmp + 1]),
								dot(coord, ctrmtx[tmp + 2]));
						} GL_ELSE{
							output_tex[x, y, z] = vec3(dot(coord, ctrmtx[tmp]),
								dot(coord, ctrmtx[tmp + 1]),
								1.0);
						}
					} GL_ELSE{
						GL_IF(bitfieldExtract(texMtxInfo, 1, 1) == 1u) {
							output_tex[x, y, z] = vec3(dot(coord, ctexmtx[3u * texgen]),
								dot(coord, ctexmtx[3u * texgen + 1u]),
								dot(coord, ctexmtx[3u * texgen + 2u]));
						} GL_ELSE {
							output_tex[x, y, z] = vec3(dot(coord, ctexmtx[3u * texgen]),
								dot(coord, ctexmtx[3u * texgen + 1u]),
								1.0);
						}
					}
					GL_BREAK;
				}

			}

			GL_IF(xfmem_dualTexInfo != 0u) {
				Uint postMtxInfo = (xfmem_pack1[(texgen)][y]);
				Uint base_index = bitfieldExtract(postMtxInfo, 0, 6);
				vec4 P0 = cpostmtx[base_index & 0x3fu];
				vec4 P1 = cpostmtx[(base_index + 1u) & 0x3fu];
				vec4 P2 = cpostmtx[(base_index + 2u) & 0x3fu];

				GL_IF(bitfieldExtract(postMtxInfo, 8, 1) != 0u)
					output_tex[x, y, z] = normalize(output_tex[x, y, z]);

				// multiply by postmatrix
				output_tex[x, y, z] = vec3(dot(P0[x, y, z], output_tex[x, y, z]) + P0[w],
					dot(P1[x, y, z], output_tex[x, y, z]) + P1[w],
					dot(P2[x, y, z], output_tex[x, y, z]) + P2[w]);
			}

			GL_IF(texgentype == 0u && output_tex[z] == 0.0) // XF_TEXGEN_REGULAR
				output_tex[x, y] = clamp(output_tex[x, y] / 2.0, vec2(-1.0, -1.0), vec2(1.0, 1.0));

			// Hopefully GPUs that can support dynamic indexing will optimize this.
			GL_SWITCH(texgen) {
				GL_CASE(0u) : { o.tex0 = output_tex; GL_BREAK; }
				GL_CASE(1u) : { o.tex1 = output_tex; GL_BREAK; }
			GL_DEFAULT: { }
			}
		}

		o.clipPos = o.pos;
		Float clipDepth = o.pos[z] * (1.0 - 1e-7);
		o.clipDist0 = clipDepth + o.pos[w];
		o.clipDist1 = -clipDepth;
		o.pos[z] = o.pos[w] * cpixelcenter[w] - o.pos[z] * cpixelcenter[z];
		o.pos[x, y] *= sign(cpixelcenter[x, y] * vec2(1.0, -1.0));
		o.pos[x, y] = o.pos[x, y] - o.pos[w] * cpixelcenter[x, y];
		GL_IF(o.pos[w] == 1.0)
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

	return shader.str();
}

std::string dolphinFragment() {
	using namespace csl::vert_430;
	using namespace csl::swizzles::all;

	Shader shader;

	// Pixel UberShader for 2 texgens, early-depth
	auto idot = declareFunc<Int,Int>( "idot", 
	[](ivec3 ix = "x", ivec3 iy = "y") {
		ivec3 tmp = ix * iy;
		GL_RETURN(tmp[x] + tmp[y] + tmp[z]);
	}, [](ivec4 ix, ivec4 iy) {
		ivec4 tmp = ix * iy;
		GL_RETURN(tmp[x] + tmp[y] + tmp[z] + tmp[w]);
	});


	return shader.str();
}