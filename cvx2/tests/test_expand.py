import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent.parent))
from cvx2.generator import generator

FIXTURE_HEADER = '''\
#include "cvx2/fallback.h"

// clang-format off
#ifndef CVX_VAL
#error "requires CVX_VAL"
#endif
#ifndef CVX_SNAME
#error "requires CVX_SNAME"
#endif
#ifndef CVX_PFX
#error "requires CVX_PFX"
#endif
// clang-format on

#include "cvx2/core.h"
#include "cvx2/flags.h"

#define FUNC(X) CVX_(CVX_PFX, X)
#define VTAB_V CVX_(CVX_SNAME, _vtabv)

struct VTAB_V
{
    CVX_VTAB_DEFINITION(CVX_VAL)
};

struct CVX_SNAME
{
    CVX_VAL *buffer;
};

enum cvx_flags FUNC(_init)(struct CVX_SNAME *self);
enum cvx_flags FUNC(__private_helper)(struct CVX_SNAME *self);
'''


def make_ctx(value_type="int", struct_name="my_thing", prefix="mt", case_name=generator.CASE_SNAKE):
    return generator.ExpandContext(
        value_type=value_type,
        key_type=None,
        struct_name=struct_name,
        prefix=prefix,
        case_fn=generator.case_fn_for(case_name),
    )


class TestExpand(unittest.TestCase):
    def test_strips_scaffolding(self):
        macros, stripped = generator.parse_local_macros(FIXTURE_HEADER)
        self.assertNotIn("cvx2/fallback.h", stripped)
        self.assertNotIn("#error", stripped)
        self.assertNotIn("#define FUNC", stripped)
        self.assertNotIn("#define VTAB_V", stripped)
        self.assertIn('#include "cvx2/core.h"', stripped)

    def test_resolves_func_and_pastes_snake_case(self):
        macros, stripped = generator.parse_local_macros(FIXTURE_HEADER)
        out = generator.expand(stripped, macros, make_ctx())
        self.assertIn("struct my_thing_vtabv", out)
        self.assertIn("struct my_thing", out)
        self.assertIn("mt_init(struct my_thing *self)", out)
        self.assertIn("int *buffer", out)
        self.assertNotIn("CVX_VAL", out)
        self.assertNotIn("CVX_SNAME", out)
        self.assertNotIn("CVX_PFX", out)
        self.assertNotIn("FUNC(", out)

    def test_resolves_camel_case_and_keeps_private_marker(self):
        macros, stripped = generator.parse_local_macros(FIXTURE_HEADER)
        out = generator.expand(stripped, macros, make_ctx(case_name=generator.CASE_CAMEL))
        self.assertIn("struct myThingVtabv", out)
        self.assertIn("mtInit(struct myThing *self)", out)
        self.assertIn("mt__PrivateHelper(struct myThing *self)", out)

    def test_resolves_upper_camel_case_and_keeps_private_marker(self):
        macros, stripped = generator.parse_local_macros(FIXTURE_HEADER)
        out = generator.expand(stripped, macros, make_ctx(case_name=generator.CASE_UPPER_CAMEL))
        self.assertIn("struct MyThingVtabv", out)
        self.assertIn("MtInit(struct MyThing *self)", out)
        self.assertIn("Mt__PrivateHelper(struct MyThing *self)", out)

    def test_value_type_never_case_converted(self):
        macros, stripped = generator.parse_local_macros(FIXTURE_HEADER)
        out = generator.expand(stripped, macros, make_ctx(value_type="char *", case_name=generator.CASE_CAMEL))
        self.assertIn("char * *buffer", out)

    def test_rewrite_self_include(self):
        text = '#include "dynamic_array.h"\n'
        out = generator.rewrite_self_include(text, "dynamic_array", "int_array.h")
        self.assertEqual(out, '#include "int_array.h"\n')


if __name__ == "__main__":
    unittest.main()
