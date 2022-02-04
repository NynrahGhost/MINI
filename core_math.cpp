#include "core_math.h"

namespace Core {

	uint64 bitOR(uint64 left, uint64 right) { return left | right; }
	uint64 bitAND(uint64 left, uint64 right) { return left & right; }
	uint64 bitXOR(uint64 left, uint64 right) { return left ^ right; }
	
	float64 add_dd(float64 left, float64 right) { return left + right; }
	float64 add_df(float64 left, float32 right) { return left + right; }
	float64 add_di(float64 left, int64 right) { return left + right; }
	float64 add_du(float64 left, uint64 right) { return left + right; }
	float64 add_fd(float32 left, float64 right) { return left + right; }
	float32 add_ff(float32 left, float32 right) {return left + right;}
	float32 add_fi(float32 left, int64 right) { return left + right; }
	float32 add_fu(float32 left, uint64 right) { return left + right; }
	float64 add_id(int64 left, float64 right) { return left + right; }
	float32 add_if(int64 left, float32 right) { return left + right; }
	int64   add_ii(int64 left, int64 right) { return left + right; }
	int64   add_iu(int64 left, uint64 right) { return left + right; }
	float64 add_ud(uint64 left, float64 right) { return left + right; }
	float32 add_uf(uint64 left, float32 right) { return left + right; }
	int64   add_ui(uint64 left, int64 right) { return left + right; }
	uint64  add_uu(uint64 left, uint64 right) { return left + right; }

	float64 sub_dd(float64 left, float64 right) { return left + right; }
	float64 sub_df(float64 left, float32 right) { return left + right; }
	float64 sub_di(float64 left, int64 right) { return left + right; }
	float64 sub_du(float64 left, uint64 right) { return left + right; }
	float64 sub_fd(float32 left, float64 right) { return left + right; }
	float32 sub_ff(float32 left, float32 right) { return left + right; }
	float32 sub_fi(float32 left, int64 right) { return left + right; }
	float32 sub_fu(float32 left, uint64 right) { return left + right; }
	float64 sub_id(int64 left, float64 right) { return left + right; }
	float32 sub_if(int64 left, float32 right) { return left + right; }
	int64   sub_ii(int64 left, int64 right) { return left + right; }
	int64   sub_iu(int64 left, uint64 right) { return left + right; }
	float64 sub_ud(uint64 left, float64 right) { return left + right; }
	float32 sub_uf(uint64 left, float32 right) { return left + right; }
	int64   sub_ui(uint64 left, int64 right) { return left + right; }
	uint64  sub_uu(uint64 left, uint64 right) { return left + right; }

	float64 mul_dd(float64 left, float64 right) { return left * right; }
	float64 mul_df(float64 left, float32 right) { return left * right; }
	float64 mul_di(float64 left, int64 right) { return left * right; }
	float64 mul_du(float64 left, uint64 right) { return left * right; }
	float64 mul_fd(float32 left, float64 right) { return left * right; }
	float32 mul_ff(float32 left, float32 right) { return left * right; }
	float32 mul_fi(float32 left, int64 right) { return left * right; }
	float32 mul_fu(float32 left, uint64 right) { return left * right; }
	float64 mul_id(int64 left, float64 right) { return left * right; }
	float32 mul_if(int64 left, float32 right) { return left * right; }
	int64   mul_ii(int64 left, int64 right) { return left * right; }
	int64   mul_iu(int64 left, uint64 right) { return left * right; }
	float64 mul_ud(uint64 left, float64 right) { return left * right; }
	float32 mul_uf(uint64 left, float32 right) { return left * right; }
	int64   mul_ui(uint64 left, int64 right) { return left * right; }
	uint64  mul_uu(uint64 left, uint64 right) { return left * right; }

	float64 div_dd(float64 left, float64 right) { return left / right; }
	float64 div_df(float64 left, float32 right) { return left / right; }
	float64 div_di(float64 left, int64 right) { return left / right; }
	float64 div_du(float64 left, uint64 right) { return left / right; }
	float64 div_fd(float32 left, float64 right) { return left / right; }
	float32 div_ff(float32 left, float32 right) { return left / right; }
	float32 div_fi(float32 left, int64 right) { return left / right; }
	float32 div_fu(float32 left, uint64 right) { return left / right; }
	float64 div_id(int64 left, float64 right) { return left / right; }
	float32 div_if(int64 left, float32 right) { return left / right; }
	int64   div_ii(int64 left, int64 right) { return left / right; }
	int64   div_iu(int64 left, uint64 right) { return left / right; }
	float64 div_ud(uint64 left, float64 right) { return left / right; }
	float32 div_uf(uint64 left, float32 right) { return left / right; }
	int64   div_ui(uint64 left, int64 right) { return left / right; }
	uint64  div_uu(uint64 left, uint64 right) { return left / right; }

	uint64 mod_uu(uint64 left, uint64 right) {return left % right;}
	int64 mod_ui(uint64 left, int64 right) { return left % right; }
	int64 mod_iu(int64 left, uint64 right) { return left % right; }
	int64 mod_ii(int64 left, int64 right) { return left % right; }
}