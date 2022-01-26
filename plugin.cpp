#include <stdio.h>

#include <gcc-plugin.h>
#include <tree.h>
#include <tree-iterator.h>
#include <print-tree.h>
#include "tree-pretty-print.h"
#include <plugin-version.h>

int plugin_is_GPL_compatible;

static void iterate_function_body(tree expr);

static void mutator(tree node) {
    if (TREE_CODE(node) == COND_EXPR || TREE_CODE(node) == TRUTH_ORIF_EXPR) {
        for (int i = 0; i < TREE_OPERAND_LENGTH(node); i++) {
            tree op = TREE_OPERAND(node, i);

            if (op == NULL) {   
                return;
            }

            if (TREE_CODE(op) == TRUTH_ORIF_EXPR) { 
                iterate_function_body(op);
                tree arg1 = TREE_OPERAND(op, 0);
                tree arg2 = TREE_OPERAND(op, 1);
                TREE_OPERAND(node, i) = build2(TRUTH_ANDIF_EXPR, TREE_TYPE(op), arg1, arg2);
            } else if (TREE_CODE(op) == COND_EXPR) {
                iterate_function_body(op);
            }
        }
    }
}

static void iterate_function_body(tree expr) {
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
            }
            mutator(stmt);
        }
    } else {
        mutator(body);
    }
}

static void finish_parse_callback(void *event_data, void *user_data) {
    tree t = (tree) event_data;
    const char* function_name = IDENTIFIER_POINTER(DECL_NAME(t));

    if (!strcmp(function_name, "main")) {
        return;
    }

    tree body = BIND_EXPR_BODY(DECL_SAVED_TREE(t));
    iterate_function_body(body);
    //debug_tree(body);
    //debug_generic_expr(body);
}

int plugin_init(struct plugin_name_args *plugin_info, struct plugin_gcc_version *version) {
    register_callback(plugin_info->base_name, PLUGIN_FINISH_PARSE_FUNCTION, finish_parse_callback, NULL);
    return 0;
}
