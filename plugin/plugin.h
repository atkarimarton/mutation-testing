#ifndef PLUGIN_HPP
#define PLUGIN_HPP

void iterate_function_body(tree);
void and_to_or_mutator(tree);
void or_to_and_mutator(tree);
void lt_to_gt(tree);
void lt_to_ge(tree);
void lt_to_le(tree);
void le_to_lt(tree);
void le_to_ge(tree);
void le_to_gt(tree);
void gt_to_le(tree);
void gt_to_lt(tree);
void gt_to_ge(tree);
void ge_to_gt(tree);
void ge_to_lt(tree);
void ge_to_le(tree);
void plus_to_minus(tree);
void minus_to_plus(tree);
void div_to_mul(tree);
void mul_to_div(tree);
void eq_to_ne(tree);
void ne_to_eq(tree);
void truthify(tree);
void falsify(tree);
void return_zero(tree);

#endif