#include <algorithm>
#include <cmath>
#include <iostream>
#include <raylib.h>

const char * shader_str = "#version 330\n\
\
in vec2 fragTexCoord;\
in vec4 fragColor;\
\
out vec4 finalColor;\
\
uniform float W = 0.0;\
uniform float H = 0.0;\
uniform float px = 0.0;\
uniform float py = 0.0;\
uniform float jx = 0.0;\
uniform float jy = 0.0;\
uniform float untPerPix = 0.0;\
uniform int colordetail = 50;\
uniform int julia = 0;\
uniform int precise = 0;\
uniform vec3 color;\
\
const int PRECISION = 5;\
const int ARRAY_SIZE = (PRECISION+1);\
const float BASE = 4294967296.0;\
const uint HALF_BASE = 2147483648u;\
\
const int ITERATIONS = 250;\
const int ITERATIONS_PRECISE = 1000;\
\n\
#define assign(x, y) {for(int assign_i=0;assign_i<=PRECISION;assign_i++){x[assign_i]=y[assign_i];}}\n\
#define zero(x) {for(int zero_i=0;zero_i<=PRECISION;zero_i++){x[zero_i]=0u;}}\n\
#define load(x, v) {float load_value=(v); if (load_value<0.0) {x[0]=1u; load_value*=-1.0;} else {x[0]=0u;} for(int load_i=1; load_i<=PRECISION; load_i++) {x[load_i]=uint(load_value); load_value-=x[load_i]; load_value*=BASE;}}\n\
#define shift(x, v) {int shift_n=(v); for(int shift_i=shift_n+1; shift_i<=PRECISION; shift_i++) {x[shift_i]=x[shift_i-shift_n];} for(int shift_i=1; shift_i<=shift_n; shift_i++) {x[shift_i]=0u;}};\n\
#define negate(x) {x[0]=(x[0]==0u?1u:0u);}\n\
#define add(a, b, r) {uint add_buffer[PRECISION+1]; bool add_pa=a[0]==0u; bool add_pb=b[0]==0u; if (add_pa==add_pb) {uint add_carry=0u; for(int add_i=PRECISION; add_i>0; add_i--) {uint add_next=0u; if(a[add_i]+b[add_i]<a[add_i]) {add_next=1u;} add_buffer[add_i]=a[add_i]+b[add_i]+add_carry; add_carry=add_next;} if(!add_pa) {add_buffer[0]=1u;} else {add_buffer[0]=0u;}} else {bool add_flip=false; for(int add_i=1; add_i<=PRECISION; add_i++) {if(b[add_i]>a[add_i]) {add_flip=true; break;} if(a[add_i]>b[add_i]) {break;}} if(add_flip) {uint add_borrow=0u; for(int add_i=PRECISION; add_i>0; add_i--) {add_buffer[add_i]=b[add_i]-a[add_i]-add_borrow; if(b[add_i]<a[add_i]+add_borrow) {add_borrow=1u;} else {add_borrow=0u;}}} else {uint add_borrow=0u; for(int add_i=PRECISION; add_i>0; add_i--) {add_buffer[add_i]=a[add_i]-b[add_i]-add_borrow; if(a[add_i]<b[add_i]||a[add_i]<b[add_i]+add_borrow) {add_borrow=1u;} else {add_borrow=0u;}}} if(add_pa==add_flip) {add_buffer[0]=1u;} else {add_buffer[0]=0u;}} assign(r, add_buffer);}\n\
#define mul(a, b, r) {uint mul_buffer[PRECISION+1]; zero(mul_buffer); uint mul_product[2*PRECISION-1]; for(int mul_i=0; mul_i<2*PRECISION-1; mul_i++) {mul_product[mul_i]=0u;} for(int mul_i=0; mul_i<PRECISION; mul_i++) {uint mul_carry=0u; for(int mul_j=0; mul_j<PRECISION; mul_j++) {uint mul_next=0u; uint mul_value=a[PRECISION-mul_i]*b[PRECISION-mul_j]; if(mul_product[mul_i+mul_j]+mul_value<mul_product[mul_i+mul_j]) {mul_next++;} mul_product[mul_i+mul_j]+=mul_value; if(mul_product[mul_i+mul_j]+mul_carry<mul_product[mul_i+mul_j]) {mul_next++;} mul_product[mul_i+mul_j]+=mul_carry; uint mul_lower_a=a[PRECISION-mul_i]&0xFFFFu; uint mul_upper_a=a[PRECISION-mul_i]>>16u; uint mul_lower_b=b[PRECISION-mul_j]&0xFFFFu; uint mul_upper_b=b[PRECISION-mul_j]>>16u; uint mul_lower=mul_lower_a*mul_lower_b; uint mul_upper=mul_upper_a*mul_upper_b; uint mul_mid=mul_lower_a*mul_upper_b; mul_upper+=mul_mid>>16u; mul_mid=mul_mid<<16u; if(mul_lower+mul_mid<mul_lower) {mul_upper++;} mul_lower+=mul_mid; mul_mid=mul_lower_b*mul_upper_a; mul_upper+=mul_mid>>16u; mul_mid=mul_mid<<16u; if(mul_lower+mul_mid<mul_lower) {mul_upper++;}; mul_carry=mul_upper+mul_next;} if(mul_i+PRECISION<2*PRECISION-1) {mul_product[mul_i+PRECISION]+=mul_carry;}} if(PRECISION > 1 && mul_product[PRECISION-2]>=HALF_BASE) {for(int mul_i=PRECISION-1; mul_i<2*PRECISION-1; mul_i++) {if(mul_product[mul_i]+1u>mul_product[mul_i]) {mul_product[mul_i]++; break;} mul_product[mul_i]++;}} for(int mul_i=0; mul_i<PRECISION; mul_i++) {mul_buffer[mul_i+1]=mul_product[2*PRECISION-2-mul_i];} if((a[0]==0u)!=(b[0]==0u)) {mul_buffer[0]=1u;}; assign(r, mul_buffer);}\n\
\
void main()\
{\
    vec2 fragPos = fragTexCoord * vec2(W, H);\
    vec3 clr = vec3(1.0);\
    float offx = -W * 0.5 * untPerPix;\
	float offy = -H * 0.5 * untPerPix;\
    if (precise > 0) {\
        uint u[ARRAY_SIZE];\
        load(u, untPerPix);\
        uint pr[ARRAY_SIZE];\
        load(pr, px);\
        uint pi[ARRAY_SIZE];\
        load(pi, py);\
        uint orr[ARRAY_SIZE];\
        load(orr, offx);\
        uint oii[ARRAY_SIZE];\
        load(oii, offy);\
\
        uint c_r[ARRAY_SIZE];\
        load(c_r, fragPos.x);\
        mul(c_r, u, c_r);\
        add(c_r, pr, c_r);\
        add(c_r, orr, c_r);\
\
        uint c_i[ARRAY_SIZE];\
        load(c_i, fragPos.y);\
        mul(c_i, u, c_i);\
        add(c_i, pi, c_i);\
        add(c_i, oii, c_i);\
\
        uint r[ARRAY_SIZE];\
        uint i[ARRAY_SIZE];\
        uint j_r[ARRAY_SIZE];\
        uint j_i[ARRAY_SIZE];\
        load(j_r, jx);\
        load(j_i, jy);\
        uint rr[ARRAY_SIZE];\
        uint ii[ARRAY_SIZE];\
        uint ri[ARRAY_SIZE];\
        uint tmp[ARRAY_SIZE];\
        if (julia > 0) {\
            assign(r, c_r);\
            assign(i, c_i);\
        }\
        for (int it = 0; it < ITERATIONS_PRECISE; ++it) {\
            mul(r, i, ri);\
            mul(r, r, rr);\
            mul(i, i, ii);\
            if (julia > 0) {\
                negate(ii);\
                add(rr, ii, r);\
                add(r, j_r, r);\
                add(ri, ri, i);\
                add(i, j_i, i);\
            } else {\
                negate(ii);\
                add(rr, ii, r);\
                add(r, c_r, r);\
                add(ri, ri, i);\
                add(i, c_i, i);\
            }\
            zero(tmp);\
            add(rr, ii, tmp);\
            if (tmp[1] > 4u) {\
                float t = (float(it) / (float(colordetail)));\
                if (t > 0.5)\
                    clr = vec3(t) - 0.5 * (vec3(1.) - color);\
                else\
                    clr = vec3(t) * color;\
                break;\
            }\
        }\
    } else {\
        float cx = px + fragPos.x * untPerPix + offx;\
        float cy = py + fragPos.y * untPerPix + offy;\
        float x = 0, y = 0, xx, yy, xy;\
        if (julia > 0) {\
            x = cx;\
            y = cy;\
        }\
        for (int i = 0; i < ITERATIONS; ++i) {\
            xy = x * y;\
            xx = x * x;\
            yy = y * y;\
            if (julia > 0) {\
                x = xx - yy + jx;\
                y = xy + xy + jy;\
            } else {\
                x = xx - yy + cx;\
                y = xy + xy + cy;\
            }\
            if (xx + yy > 4) {\
                float t = (float(i) / (float(colordetail)));\
                if (t > 0.5)\
                    clr = vec3(t) - 0.5 * (vec3(1.) - color);\
                else\
                    clr = vec3(t) * color;\
                break;\
            }\
        }\
    }\
\
    finalColor = vec4(clr, 1.0);\
}\
";

#define RAND_FLOAT static_cast <float> (rand()) / (static_cast <float> (RAND_MAX) + 1.0f)

#if defined(PLATFORM_DESKTOP)
    #define GLSL_VERSION            330
#else
    #define GLSL_VERSION            100
#endif

#define WIN_W 1024
#define WIN_H 1024
#define INIT_SPAN 8.0

void input(float w, float h, double& x, double& y, double& x2, double& y2, double& unitsPerPix, int& julia, int& precise, int& colordetail, Vector3& color) {
	static Vector2 lastGrabPos;
	static bool grabbing = false;
	auto mpos = GetMousePosition();
	auto wheel = GetMouseWheelMove();
	auto mx = x + (mpos.x - w * 0.5) * unitsPerPix;
	auto my = y + (mpos.y - h * 0.5) * unitsPerPix;
    auto shift = IsKeyDown(KEY_LEFT_SHIFT);
    auto ctrl = IsKeyDown(KEY_LEFT_CONTROL);
	if (IsMouseButtonPressed(0)) {
		lastGrabPos = GetMousePosition();
		grabbing = true;
	} else if (IsMouseButtonReleased(0)) {
		grabbing = false;
	} else if (grabbing) {
		x += (lastGrabPos.x - mpos.x) * unitsPerPix;
		y += (lastGrabPos.y - mpos.y) * unitsPerPix;
		lastGrabPos = GetMousePosition();
	}
	if (wheel > 0) {
        if (ctrl) color = Vector3{RAND_FLOAT, RAND_FLOAT, RAND_FLOAT};
        else if (shift) colordetail += std::max(ceil(colordetail / 2.0), 1.0);
        else unitsPerPix *= 0.75;
    } else if (wheel < 0) {		
        if (ctrl) color = Vector3{RAND_FLOAT, RAND_FLOAT, RAND_FLOAT};
        else if (shift) colordetail -= std::max(ceil(colordetail / 2.0), 1.0);
        else unitsPerPix *= 1.25;
    }
	if (wheel != 0) {
		auto newMX = x + (mpos.x - w * 0.5) * unitsPerPix;
		auto newMY = y + (mpos.y - h * 0.5) * unitsPerPix;
		x += (mx - newMX);
		y += (my - newMY);
	}
    colordetail = std::clamp(colordetail, 1, precise ? 1000 : 250);
	if (IsMouseButtonDown(1)) {
		x2 = mx;
		y2 = my;
		julia = 1;
	}
	if (IsMouseButtonPressed(2)) {
		x = 0;
		y = 0;
		unitsPerPix = INIT_SPAN / w;
		julia = 0;
	}
    //precise = IsKeyDown(KEY_SPACE);
    if (IsKeyPressed(KEY_SPACE)) {
        precise = !precise;    
    }
    if (IsMouseButtonPressed(KEY_ESCAPE))
        CloseWindow();    
}

int main(int argc, char *argv[]) {

	InitWindow(0, 0, "mandelbrot");
	SetTargetFPS(60);
	ToggleFullscreen();
	float w = GetScreenWidth(), h = GetScreenHeight(); 

	Shader shader = LoadShaderFromMemory(nullptr, shader_str);
    SetShaderValue(shader, GetShaderLocation(shader, "W"), &w, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, GetShaderLocation(shader, "H"), &h, SHADER_UNIFORM_FLOAT);
	Image imBlank = GenImageColor(w, h, BLANK);
    Texture2D texture = LoadTextureFromImage(imBlank);

	double x = 0, y = 0, x2 = 0, y2 = 0;
	double unitPerPix = INIT_SPAN / w;
    float fx, fy, fx2, fy2, fupp;
	int colordetail = 75;
	int julia = 0;
	int precise = 0;
    Vector3 color = Vector3{1.0f, 0, 0};

	while (!WindowShouldClose()) {
		BeginDrawing();
		BeginShaderMode(shader);
		DrawTexture(texture, 0, 0, WHITE);
		EndShaderMode();
		EndDrawing();
		input(w, h, x, y, x2, y2, unitPerPix, julia, precise, colordetail, color);
        fx = x; fy = y; fx2 = x2; fy2 = y2; fupp = unitPerPix;
	    SetShaderValue(shader, GetShaderLocation(shader, "px"), &fx, SHADER_UNIFORM_FLOAT);
	    SetShaderValue(shader, GetShaderLocation(shader, "py"), &fy, SHADER_UNIFORM_FLOAT);
	    SetShaderValue(shader, GetShaderLocation(shader, "untPerPix"), &fupp, SHADER_UNIFORM_FLOAT);
		SetShaderValue(shader, GetShaderLocation(shader, "jx"), &fx2, SHADER_UNIFORM_FLOAT);
		SetShaderValue(shader, GetShaderLocation(shader, "jy"), &fy2, SHADER_UNIFORM_FLOAT);
        SetShaderValue(shader, GetShaderLocation(shader, "colordetail"), &colordetail, SHADER_UNIFORM_INT);
		SetShaderValue(shader, GetShaderLocation(shader, "julia"), &julia, SHADER_UNIFORM_INT);
		SetShaderValue(shader, GetShaderLocation(shader, "precise"), &precise, SHADER_UNIFORM_INT);
		SetShaderValue(shader, GetShaderLocation(shader, "color"), &color, SHADER_UNIFORM_VEC3);
	}

    return 0;
}