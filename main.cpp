#include <iostream>

#include "AlgebraFunctions.h"
#include "Layouts.h"
#include "Samplers.h"
#include "BuildingBlocks.h"
#include "Shaders.h"


void srt1();
void srt2();
void firstTest();

template<typename A, typename B, typename = std::enable_if_t< 
	NoBools<A, B> && ( EquaMat<A,B> || IsScalar<A> || IsScalar<B> )
> > 
ArithmeticBinaryReturnType<A,B> testAdd(const A& a, const B& b) {

}

int main()
{

	//firstTest();
	//testFuns();
	//testBlocks();
	//srt1();
	srt2();

	return 0;
}

void firstTest() {
	{
		using namespace fs;

		FragmentShader shader;

		auto test = makeF("fun", [](Float f, Float g) {
			return Float(f + g);
		}, "f", "g");


		auto test2 = makeF("fun", [](Float f, Float g) {
			return f + g;
		}, "f", "g");


		GL_STRUCT(Tri,
			(Float) angle,
			(vec3) center,
			(mat4x4) proju
		);

		GL_STRUCT(MegaTri,
			(Tri) triA,
			(Tri) triB,
			(Tri) triC
		);

		GL_STRUCT(MegaTron,
			(Tri) triA,
			(MegaTri) mega,
			(Tri) triC
		);


		In<vec3> normal("normal"), position("position"), color("color");
		Uniform<vec3> lightPos("lightPos");
		Out<vec4> outColor("outColor");
		Uniform<Tri> triangle("triangle");
		Uniform<MegaTri, Layout<Location<3>, Binding<0> >> megatron;

		auto foo = makeF("proj", [](In<mat4> proj, In<vec3> point) { return proj * vec4(point, 1.0); }, "proj", "point");
		auto goo = makeF("goo", [](vec3 a, vec3 b) { return determinant(mat3(a, b, a)); });

		shader.main([&] {

			vec3 rotatedCenter = ( 0.4 + triangle.center * triangle.angle *std::sin(1.0) ) / 9.2 << "rotated";
			vec3 L = normalize(lightPos - position) << "L";
			vec3 diff = L[x] * color * max(dot(normal, L), 0.0) + gl_FragCoord[x, y, z] * L[x] << "diff";
			//diff = dot(diff, triangle.center)*diff;

			mat4 mm = mat4(lightPos[x]) << "";
			mat4 mmm = mat4(1.0) << "";

			Int n("n"), m("m");
			n = m++;
			n = ++m;
			diff = diff + diff[x] * diff + megatron.triB.proju[0][x, y, z];

			GL_FOR(Int a(0, "a"); a < 5; a++) {
				Bool myb;
				GL_IF((!myb && myb) || (False && True)) {
					diff[x] = goo(L, diff);
				} GL_ELSE_IF(True) {
					vec4 t = vec4(L, 1.0) << "t";
					diff[x] = goo(L, foo(mat4(t, t, t, t), diff)[r, b, g])[x];
				}

			}

			outColor = vec4(color[x, z], color[x, y]);

		});

		std::cout << shader.getStr() << std::endl;


	}

	{
		using namespace vs;
		VertexShader shader;

		GL_STRUCT(MyStruct,
			(Float) f,
			(Bool) b
		);

		Uniform<MyStruct, Layout<Binding<3>> > structor("structor");

		shader.main([&] {
			gl_Position[x, y, z] = vec3(structor.f, 1.0, structor.f) / gl_Position[z];
		});

		std::cout << shader.getStr() << std::endl;
	}
}

void testFuns() {
	auto funct1 = makeF("mul1", [](vec2 a, Float b) { vec2 c; c = b * a; return c; });
	auto funct2 = makeF("mul2", [](vec2 a, Float b) { return b * a; });
	auto funct3 = makeF("mul3", [](vec2 a, Float b) { vec2 c; c = b * a; });
	auto funct4 = makeF("mul4", [](vec2 a, Float b) { vec2 c("c"); c = b * a; }, "bkah", "bloh");
	auto funct5 = makeF("mul5", [](vec2 a, Float b) { GL_RETURN b * a; }, "bkah", "bloh");

	vec2 vv("v2");
	Float ff("ff");
	vv = funct1(vv, ff);
	vv = funct2(vv, ff);
	funct3(vv, ff);
	funct4(vv, ff);
	vv = vec2(funct5(vv, ff)[y], funct5(vv, ff)[x]);
}

void testBlocks() {
	std::cout << std::endl << "begin loop tests" << std::endl << std::endl;

	GL_FOR(Int a("a"); a < 5; ++a) {
		Int c("c");
		Continue();
		GL_FOR(Int b(0, "b"); b < a; ++b) {
			++c;
		}
		Int e("e");
	}
	//vec4(0);
	vec3 v1("v1");
	vec3 v2("v2");
	GL_IF(all(lessThanEqual(v1, v2))) {
		Int c("c");
		//c < c;

		ivec3 v2("v");

		v2[x, z] = ivec2(c, c);

		ivec4 v4("v4");

		v4 = ivec4(ivec2(c, c), ivec2(c, c));

		(v4[x, y, z]) = (v4[b, b, g]);

		++c;

		//v4 = ivec4(ivec2(), ivec2());

		v4;
		//v4 = vec4();
		//vec4 vv = vec4(0 );


	} GL_ELSE_IF(all(greaterThanEqual(v2, v1))) {
		GL_IF(any(lessThan(v2, v1))) {
		}
		vec3 c("c");
		Float f = c[z] + length(c) << "f";

	} GL_ELSE{
		vec3 c("c");
	c[x] = length(c);
	c[x] = inversesqrt(c[x]) + sqrt(c[y]) + c[x] * c[x];
	c[z, x] += vec2(exp(c[x]), log(c[y]));
	c[x, y] = cos(c[z, z] + c[y, y] * c[z] + c[x] * c[x, x]);
	mat3 m, j;
	mat3 & mm = m;
	mm = j;

	j = c[z] * transpose(m);
	//c[x] = ( ( c[x] + c[y] ) + c[x] );
	//c[x] += c[y];
	//c[x, y] += c[x, x];
	//ivec2 ii;
	}

}





/// Tests

void srt1(){
	
	using namespace vs;
	VertexShader shader0;
	
	// Output: UV coordinates
	/* out INTERFACE {
	 vec2 uv;
	 } Out ;*/
	Out<vec2> Outuv("uv");
	
	shader0.main([&]{
		// vec2 temp = 2.0 * vec2(gl_VertexID == 1, gl_VertexID == 2)
		vec2 temp = Float(2.0) * vec2(gl_VertexID == Int(1), gl_VertexID == Int(2)) << "";
		Outuv = temp;
		// gl_Position[x,y] = 2.0 * temp - 1.0;
		gl_Position[x,y] = Float(2.0) * temp - vec2(1.0);
		gl_Position[z,w] = vec2(1.0);
		
	});
	
	std::cout << shader0.getStr() << std::endl;
	
	using namespace fs;
	FragmentShader shader1;
	
	// Input: UV coordinates
	/* in INTERFACE {
	 vec2 uv;
	 } In ;
	 */
	In<vec2> Inuv("uv");
	Out<vec3, Layout<Location<0>>> fragColor("fragColor");
	// const float M_PI = 3.14159;
	Float pi(3.14159, "M_PI"); // Is constness possible?
	// Uniform<sampler2D> screenTexture("screenTexture");
	// Had to remove the sampler (no myName() implemented and other things)
	
	shader1.main([&]{
		// fragColor = texture(screenTexture, cos(pi*Inuv[y]))[r,b,g];
		// Had to remove the texture() call.
		fragColor = vec3(0.0,0.0,cos(pi*Inuv[y]))[r,b,g];
	});
	
	std::cout << shader1.getStr() << std::endl;
}

void srt2(){
	using namespace fs;
	FragmentShader shader1;
	
	In<vec2> Inuv("uv");
	Uniform<mat4> clipToWorld("clipToWorld");
	Uniform<vec3> viewPos("viewPos");
	Uniform<vec3> lightDirection("lightDirection");
	
	Out<vec3> fragColor("fragColor");
	Float groundRadius = Float(6371e3) << "groundRadius";
	Float topRadius = Float(6471e3) << "topRadius";
	Float sunIntensity = Float(20.0) << "sunIntensity";
	vec3 sunColor = vec3(1.474, 1.8504, 1.91198) << "sunColor";
	vec3 kRayleigh = vec3(5.5e-6, 13.0e-6, 22.4e-6) << "kRayleigh";
	Float kMie = Float(21e-6) << "kMie";
	Float heightRayleigh = Float(8000.0) <<"heightRayleigh";
	Float heightMie = Float(1200.0) <<"heightMie";
	Float gMie = Float(0.758) << "gMie";
	
	Float sunAngularRadius = Float(0.04675) << "sunAngularRadius";
	Float sunAngularRadiusCos = Float(0.998) << "sunAngularRadiusCos";
	
	#define SAMPLES_COUNT 16
	#define M_PI 3.14159265358979323846

	auto intersects = makeF("intersects", [](vec3 rayOrigin, vec3 rayDir, Float radius, Out<vec2> roots){
		
		Float aa = dot(rayDir,rayDir) << "a";
		Float bb = dot(rayOrigin, rayDir) << "b";
		Float cc = (dot(rayOrigin, rayOrigin) - radius*radius)  << "c";
		Float delta = bb*bb - aa*cc << "delta";
		/* Early return doesn't work?
		 GL_IF(delta < Float(0.0)){
		 GL_RETURN False;
		 }*/
		Float dsqrt = sqrt(delta) << "";
		// The unary - doesn't work :(
		roots = -1.0 * vec2(bb,bb) + dsqrt * vec2(-1.0, 1.0);
		GL_RETURN Bool(delta > 0.0);
	});
	
	
	auto rayleighPhase = makeF("rayleighPhase", [](Float cosAngle){
		const Float k = Float(1.0/(4.0*M_PI)) << "k";
		return k * Float(3.0/4.0) * (Float(1.0) + cosAngle*cosAngle);
	}, "cosAngle");
	
	auto miePhase = makeF("miePhase", [gMie](Float cosAngle){
		const Float k = 1.0/(4.0*M_PI);
		Float g2 = gMie*gMie << "g2";
		return k * Float(3.0) * (Float(1.0)-g2) / (Float(2.0) * (Float(2.0) + g2)) * (Float(1.0) + cosAngle*cosAngle) / pow(Float(1.0) + g2 - Float(2.0) * gMie * cosAngle, Float(3.0/2.0));
	});
	
	auto computeRadiance = makeF("computeRadiance", [&](vec3 rayOrigin, vec3 rayDir, vec3 sunDir){
		vec2 interTop("interTop"), interGround("interGround");
		Bool didHitTop = intersects(rayOrigin, rayDir, topRadius, interTop) << "didHitTop";
		// Here there is an early return usually.
		Bool didHitGround = intersects(rayOrigin, rayDir, groundRadius, interGround) << "didHitGround";
		// Missing min() et ternary.
		Float distanceToInter("distanceToInter");
		GL_IF(didHitGround){
			distanceToInter = interGround[x];
		} GL_ELSE {
			distanceToInter = 0.0;
		}
		Float stepSize = (distanceToInter - interTop[x])/Float(SAMPLES_COUNT) << "stepSize";
		Float cosViewSun = dot(rayDir, sunDir) << "cosViewSun";
		Float rayleighDist = Float(0.0) << "rayleighDist";
		Float mieDist = Float(0.0) << "mieDist";
		vec3 rayleighScatt = vec3(0.0) << "rayleighScatt";
		vec3 mieScatt = vec3(0.0) << "mieScatt";
		vec3 transmittance = vec3(0.0) << "transmittance";
		
		GL_FOR(Int i(0,"i"); i < SAMPLES_COUNT; ++i){
			vec3 currPos = rayOrigin + (Float(i) + 0.5) * stepSize * rayDir << "currPos";
			Float currHeight = length(currPos) - groundRadius << "currHeight";
			GL_IF(i == SAMPLES_COUNT-1 && currHeight < 0.0){
				currHeight = 0.0;
			}
			// Had to wrap - -> -1.0 -> Float(-1.0)
			Float rayleighStep = exp(-1.0*currHeight/heightRayleigh) * stepSize << "rayleighStep";
			Float mieStep = exp(-1.0*currHeight/heightMie) * stepSize << "mieStep";
			rayleighDist += rayleighStep;
			mieDist += mieStep;
			// Couldn't mix Float and vec3: kRayleigh*rayleighDist
			vec3 directAttenuation = exp(-1.0*(kMie*mieDist + kRayleigh * rayleighDist)) << "directAttenuation";
			// Missing parenthesis in the generated code.
			Float relativeHeight = (length(currPos) - groundRadius) / (topRadius - groundRadius) << "relativeHeight";
			Float relativeCosAngle = -0.5*sunDir[y]+0.5 << "relativeCosAngle";
			// same thing, can't mix float and vec2 when creating in-expression vec2.
			vec2 attenuationUVs = vec2(relativeHeight, relativeCosAngle) << "attenuationUVs";
			// attenuationUVs *= Float(1.0-1.0/512.0);
			attenuationUVs += Float(0.5/512.0);
			// vec3 secondaryAttenuation = texture(screenTexture, attenuationUVs).rgb;
			vec3 secondaryAttenuation = attenuationUVs[x,y,x];
			vec3 attenuation = directAttenuation * secondaryAttenuation << "attenuation";
			rayleighScatt += rayleighStep * attenuation;
			mieScatt += mieStep * attenuation;
			transmittance += directAttenuation;
		}
		
		vec3 rayleighParticipation = kRayleigh * rayleighPhase(cosViewSun) *  rayleighScatt << "rayleighParticipation";
		vec3 mieParticipation = kMie * miePhase(cosViewSun) * mieScatt << "mieParticipation";
		
		vec3 sunRadiance = vec3(0.0) << "sunRadiance";
		Bool didHitGroundForward = (didHitGround && interGround[y] > Float(0)) << "didHitGroundForward";
		GL_IF(!didHitGroundForward && dot(rayDir, sunDir) > sunAngularRadiusCos){
			sunRadiance = sunColor / (Float(M_PI) * sunAngularRadius * sunAngularRadius);
		}
		
		return sunIntensity * (rayleighParticipation + mieParticipation) + sunRadiance + transmittance;
	});
	
	
	shader1.main([&]{
		// This one causes an error on createDummy.
		//vec4 clipVertex = vec4(Float(-1.0)+Float(2.0)*Inuv, 0.0, 1.0);
		vec4 clipVertex = vec4(-1.0+2.0*Inuv, 0.0, 1.0) << "clipVertex";
		vec3 viewRay = normalize((clipToWorld * clipVertex)[x,y,z]) << "viewRay";
		vec3 planetSpaceViewPos = viewPos + vec3(0.0, 6371e3, 0.0) + vec3(0.0,1.0,0.0) << "planetSpaceViewPos";
		vec3 atmosphereColor = computeRadiance(planetSpaceViewPos, viewRay, lightDirection) << "atmosphereColor";
		fragColor = atmosphereColor;
	});
	
	std::cout << shader1.getStr() << std::endl;
}

