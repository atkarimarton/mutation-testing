#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <string>
#include <algorithm>

#include <gcc-plugin.h>
#include <tree.h>
#include <tree-iterator.h>
#include <print-tree.h>
#include "tree-pretty-print.h"
#include <plugin-version.h>
#include <map>

#include "plugin.h"

int plugin_is_GPL_compatible;

std::set<tree_code> TO_SEARCH{TRUTH_ANDIF_EXPR, TRUTH_AND_EXPR, TRUTH_ORIF_EXPR, TRUTH_OR_EXPR, RETURN_EXPR,
                              MODIFY_EXPR,};
std::set<tree_code> LOGICAL_OPERATORS{TRUTH_ANDIF_EXPR, TRUTH_AND_EXPR, TRUTH_ORIF_EXPR, TRUTH_OR_EXPR};
std::set<tree_code> AND_EXPR{TRUTH_ANDIF_EXPR, TRUTH_AND_EXPR};
std::set<tree_code> OR_EXPR{TRUTH_ORIF_EXPR, TRUTH_OR_EXPR};
std::vector<std::string> function_names;
std::string result_directory;
std::string target_function;
std::string rule;

bool collect_function_names = false;
int exit_code = 1;
int desired_position;
int position;

typedef void (*mutator)(tree node);

mutator mut;

class Mutator {
public:
    static void swap(tree node, const std::set<tree_code> &from, tree_code to) {
        if (TREE_CODE(node) == DECL_EXPR) {
            tree child = TREE_OPERAND(node, 0);
            if (TREE_CODE(child) == VAR_DECL && DECL_INITIAL(child) != nullptr &&
                from.find(TREE_CODE(DECL_INITIAL(child))) != from.end()) {
                if (position == desired_position) {
                    tree &init_expr = DECL_INITIAL(child);
                    change_node_type(init_expr, to);
                }
                position++;
            }
        }

        for (int i = 0; i < TREE_OPERAND_LENGTH(node); i++) {
            tree &op = TREE_OPERAND(node, i);

            if (op == nullptr) {
                return;
            }

            if (from.find(TREE_CODE(op)) != from.end()) {
                if (position == desired_position) {
                    change_node_type(op, to);
                }
                position++;
            }

            iterate_function_body(op);
        }
    }

    static void swap(tree node, tree_code from, tree_code to) {
        if (TREE_CODE(node) == DECL_EXPR) {
            tree child = TREE_OPERAND(node, 0);
            if (TREE_CODE(child) == VAR_DECL && DECL_INITIAL(child) != nullptr &&
                TREE_CODE(DECL_INITIAL(child)) == from) {
                if (position == desired_position) {
                    tree &init_expr = DECL_INITIAL(child);
                    change_node_type(init_expr, to);
                }
                position++;
            }
        }

        for (int i = 0; i < TREE_OPERAND_LENGTH(node); i++) {
            tree &op = TREE_OPERAND(node, i);

            if (op == nullptr) {
                return;
            }

            if (TREE_CODE(op) == from) {
                if (position == desired_position) {
                    change_node_type(op, to);
                }
                position++;
            }
            iterate_function_body(op);
        }
    }

    static void swap2(tree node) {
        if (TREE_CODE(node) == DECL_EXPR) {
            tree child = TREE_OPERAND(node, 0);
            tree &initial = DECL_INITIAL(child);

            if (TREE_CODE(child) == VAR_DECL && initial != nullptr) {
                if (TREE_CODE(initial) == RDIV_EXPR || TREE_CODE(initial) == TRUNC_DIV_EXPR) {
                    if (position == desired_position) {
                        change_node_type(initial, MULT_EXPR);
                    }

                    position++;
                } else if (TREE_CODE(initial) == FIX_TRUNC_EXPR) {
                    swap2(initial);
                }
            }
        }

        for (int i = 0; i < TREE_OPERAND_LENGTH(node); i++) {
            tree &op = TREE_OPERAND(node, i);

            if (op == nullptr) {
                return;
            }

            if (TREE_CODE(op) == RDIV_EXPR || TREE_CODE(op) == TRUNC_DIV_EXPR) {
                if (position == desired_position) {
                    change_node_type(op, MULT_EXPR);
                }
                position++;
            }
            iterate_function_body(op);
        }
    }

    static void swap(tree node, int val) {
        if (TREE_CODE(node) == COND_EXPR) {
            if (position == desired_position) {
                TREE_OPERAND(node, 0) = build_int_cst(integer_type_node, val);
                exit_code = 0;
            }

            position++;

            for (int i = 1; i < TREE_OPERAND_LENGTH(node); i++) {
                tree op = TREE_OPERAND(node, i);

                if (op != nullptr) {
                    iterate_function_body(op);
                }
            }
        }
    }

    static void change_node_type(tree &parent, tree_code to) {
        tree arg1 = TREE_OPERAND(parent, 0);
        tree arg2 = TREE_OPERAND(parent, 1);
        parent = build2(to, TREE_TYPE(parent), arg1, arg2);
        exit_code = 0;
    }
};

void and_to_or_mutator(tree node) {
    Mutator::swap(node, AND_EXPR, TRUTH_ORIF_EXPR);
}

void or_to_and_mutator(tree node) {
    Mutator::swap(node, OR_EXPR, TRUTH_ANDIF_EXPR);
}

void lt_to_gt(tree node) {
    Mutator::swap(node, LT_EXPR, GT_EXPR);
}

void lt_to_le(tree node) {
    Mutator::swap(node, LT_EXPR, LE_EXPR);
}

void le_to_lt(tree node) {
    Mutator::swap(node, LE_EXPR, LT_EXPR);
}

void le_to_ge(tree node) {
    Mutator::swap(node, LE_EXPR, GE_EXPR);
}

void gt_to_lt(tree node) {
    Mutator::swap(node, GT_EXPR, LT_EXPR);
}

void gt_to_ge(tree node) {
    Mutator::swap(node, GT_EXPR, GE_EXPR);
}

void ge_to_gt(tree node) {
    Mutator::swap(node, GE_EXPR, GT_EXPR);
}

void ge_to_le(tree node) {
    Mutator::swap(node, GE_EXPR, LE_EXPR);
}

void plus_to_minus(tree node) {
    Mutator::swap(node, PLUS_EXPR, MINUS_EXPR);
}

void minus_to_plus(tree node) {
    Mutator::swap(node, MINUS_EXPR, PLUS_EXPR);
}

void div_to_mul(tree node) {
    Mutator::swap2(node);
}

void eq_to_ne(tree node) {
    Mutator::swap(node, EQ_EXPR, NE_EXPR);
}

void ne_to_eq(tree node) {
    Mutator::swap(node, NE_EXPR, EQ_EXPR);
}

void lt_to_ge(tree node) {
    Mutator::swap(node, LT_EXPR, GE_EXPR);
}

void le_to_gt(tree node) {
    Mutator::swap(node, LE_EXPR, GT_EXPR);
}

void gt_to_le(tree node) {
    Mutator::swap(node, GT_EXPR, LE_EXPR);
}

void ge_to_lt(tree node) {
    Mutator::swap(node, GE_EXPR, LT_EXPR);
}

void truthify(tree node) {
    Mutator::swap(node, 1);
}

void falsify(tree node) {
    Mutator::swap(node, 0);
}

void iterate_function_body(tree expr) {
    tree body;

    if (TREE_CODE(expr) == BIND_EXPR) {
        body = BIND_EXPR_BODY(expr);
    } else {
        body = expr;
    }

    if (TREE_CODE(body) == STATEMENT_LIST) {
        for (tree_stmt_iterator i = tsi_start(body); !tsi_end_p(i); tsi_next(&i)) {
            tree stmt = tsi_stmt(i);

            if (TREE_CODE(stmt) == BIND_EXPR || TREE_CODE(stmt) == STATEMENT_LIST) {
                iterate_function_body(stmt);
            } else {
                mut(stmt);
            }
        }
    } else {
        mut(body);
    }
}

void parse_plugin_arguments(const plugin_name_args &plugin_info) {
    std::map<std::string, mutator> ruleMap{
            {"and_to_or",     and_to_or_mutator},
            {"or_to_and",     or_to_and_mutator},
            {"lt_to_gt",      lt_to_gt},
            {"lt_to_le",      lt_to_le},
            {"lt_to_ge",      lt_to_ge},
            {"le_to_lt",      le_to_lt},
            {"le_to_gt",      le_to_gt},
            {"le_to_ge",      le_to_ge},
            {"gt_to_lt",      gt_to_lt},
            {"gt_to_ge",      gt_to_ge},
            {"gt_to_le",      gt_to_le},
            {"ge_to_gt",      ge_to_gt},
            {"ge_to_lt",      ge_to_lt},
            {"ge_to_le",      ge_to_le},
            {"plus_to_minus", plus_to_minus},
            {"minus_to_plus", minus_to_plus},
            {"div_to_mul",    div_to_mul},
            {"eq_to_ne",      eq_to_ne},
            {"ne_to_eq",      ne_to_eq},
            {"truthify",      truthify},
            {"falsify",       falsify},
    };

    for (int i = 0; i < plugin_info.argc; i++) {
        std::string key = plugin_info.argv[i].key;
        if (key == "rule") {
            rule = plugin_info.argv[i].value;
            mut = ruleMap[rule];
        } else if (key == "target_function") {
            target_function = plugin_info.argv[i].value;
        } else if (key == "collect_function_names") {
            collect_function_names = true;
        } else if (key == "resultdir") {
            result_directory = plugin_info.argv[i].value;
        } else if (key == "position") {
            desired_position = strtol(plugin_info.argv[i].value, nullptr, 10);
        }
    }
}

void finish_parse_callback(void *event_data, void *user_data) {
    tree t = (tree) event_data;
    std::string function_name = IDENTIFIER_POINTER(DECL_NAME(t));

    if (function_name == "main") {
        return;
    } else if (function_name == target_function) {
        tree body = BIND_EXPR_BODY(DECL_SAVED_TREE(t));

//        debug_tree(body);
//        debug_generic_expr(body);
        iterate_function_body(body);
//        debug_tree(body);
//        debug_generic_expr(body);

        if (exit_code == 0) {
            std::string filename =
                    result_directory + target_function + '_' + rule + '_' + std::to_string(desired_position);
            FILE *fp = fopen(&filename[0], "w");
            print_generic_stmt(fp, body);
            fclose(fp);
        }
    }

    if (collect_function_names) {
        function_names.push_back(function_name);
    }
}

void plugin_finish_callback(void *event_data, void *user_data) {
    if (collect_function_names) {
        std::ofstream filestream;
        filestream.open("tmp/function_names.txt");
        if (!filestream) {
            std::cout << "File not created!\n";
        } else {
            for (const auto &it: function_names) {
                filestream << it << std::endl;
            }
            filestream.close();
        }
    }
    exit(exit_code);
}

int plugin_init(struct plugin_name_args *plugin_info, struct plugin_gcc_version *version) {
    parse_plugin_arguments(*plugin_info);

    register_callback(plugin_info->base_name, PLUGIN_FINISH_PARSE_FUNCTION, finish_parse_callback, nullptr);
    register_callback(plugin_info->base_name, PLUGIN_FINISH, plugin_finish_callback, nullptr);
    return 0;
}
