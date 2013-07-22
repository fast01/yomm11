// -*- compile-command: "make runtests" -*-

// tests.cpp
// Copyright (c) 2013 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "multimethods.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

#include "util/join.hpp"

using namespace std;
using namespace multimethods;
using boost::dynamic_bitset;

#define test(exp, res) _test(__FILE__, __LINE__, #exp, exp, #res, res)
#define testx(exp, res) _test(__FILE__, __LINE__, #exp, exp, 0, res)

namespace {
  int success, failure;
}

using methods = vector<method_base*>;

template<class T1, class T2>
bool _test(const char* file, int line, const char* test, const T1& got, const char* expected_expr, const T2& expected) {
  string ee;
  if (expected_expr) {
    ee = expected_expr;
  } else {
    ostringstream os;
    os << expected;
    ee = os.str();
  }
  bool ok = got == expected;
  if (ok) {
    cout << setw(3) << test << " returns " << ee << ", ok.\n";
    ++success;
  } else {
    cout << file << ":" << line << ": error: " << test << ": " << got << ", expected " << ee << ".\n";
    ++failure;
  }
  return ok;
}

#define DO void CONCAT(fun, __LINE__)(); int CONCAT(var, __LINE__) = (CONCAT(fun, __LINE__)(), 1); void CONCAT(fun, __LINE__)()

#define show(e) #e << " = " << (e)

template<class Ex>
bool throws(function<void()> fun) {
  try {
    fun();
  } catch (Ex) {
    return true;
  } catch (...) {
  }

  return false;
}

DO {
  cout << boolalpha;
}

namespace single_inheritance {

#include "animals.hpp"

  static_assert(
    is_same<
    typename extract_virtuals<virtual_<Animal>&, const virtual_<Animal>&>::type,
    virtuals<Animal, Animal>
    >::value, "not ok !!!");

  static_assert(
    is_same<
    typename extract_virtuals<virtual_<Animal>&, int, virtual_<Animal>&>::type,
    virtuals<Animal, Animal>
    >::value, "not ok !!!");
  
  static_assert(
    is_same<
    extract_method_virtuals<
    void(int, virtual_<Animal>&, char, const virtual_<Animal>&),
    void(int, Cow&, char, const Wolf&)
    >::type,
    virtuals<Cow, Wolf> >::value, "extraction of virtual method arguments");

  DO {
    cout << "\nClass registration" << endl;
    test(mm_class_of<Cow>::the().bases.size(), 1);
    test(mm_class_of<Cow>::the().bases[0] == &mm_class_of<Herbivore>::the(), true);
    test(mm_class_of<Animal>::the().specs.size(), 2);
    test(mm_class_of<Animal>::the().specs[0] == &mm_class_of<Herbivore>::the(), true);
    test(mm_class_of<Animal>::the().specs[1] == &mm_class_of<Carnivore>::the(), true);

    test(mm_class_of<Animal>::the().root, mm_class_of<Animal>::the().root);
    test(mm_class_of<Herbivore>::the().root, mm_class_of<Animal>::the().root);
    test(mm_class_of<Carnivore>::the().root, mm_class_of<Animal>::the().root);
    test(mm_class_of<Cow>::the().root, mm_class_of<Animal>::the().root);
    test(mm_class_of<Wolf>::the().root, mm_class_of<Animal>::the().root);
  }
}

namespace slot_allocation_tests {

  DO {
    cout << "\n--- Slot allocation." << endl;
  }

  struct X : selector {
    MM_CLASS(X);
    X() { MM_INIT(); }
  };

  MULTIMETHOD(m_x, int(const virtual_<X>&));

  struct A : X {
    MM_CLASS(A, X);
    A() { MM_INIT(); }
  };

  MULTIMETHOD(m_a, int(const virtual_<A>&));

  struct B : virtual A {
    MM_CLASS(B, A);
    B() { MM_INIT(); }
  };

  MULTIMETHOD(m_b, int(const virtual_<B>&));

  struct C : virtual A {
    MM_CLASS(C, A);
    C() { MM_INIT(); }
  };

  MULTIMETHOD(m_c, int(const virtual_<C>&));

  struct D : virtual A {
    MM_CLASS(D, A);
    D() { MM_INIT(); }
  };

  MULTIMETHOD(m_d, int(const virtual_<D>&));

  struct BC : B, C {
    MM_CLASS(BC, B, C);
    BC() { MM_INIT(); }
  };

  MULTIMETHOD(m_bc, int(const virtual_<BC>&));

  struct CD : C, D {
    MM_CLASS(CD, C, D);
    CD() { MM_INIT(); }
  };

  MULTIMETHOD(m_cd, int(const virtual_<CD>&));

  struct Y : virtual X {
    MM_CLASS(Y, X);
    Y() { MM_INIT(); }
  };

  MULTIMETHOD(m_y, int(const virtual_<Y>&));

  DO {
    // Init multimethod implementations; this is normally done when the
    // first method is added.
    m_x.the();
    m_a.the();
    m_b.the();
    m_c.the();
    m_bc.the();
    m_d.the();
    m_cd.the();
    m_y.the();

    hierarchy_initializer init(mm_class_of<X>::the());

    init.collect_classes();
    test( init.nodes.size(), 8);
    test( init.nodes.size(), 8);
    test( init.nodes[0], &mm_class_of<X>::the() );
    test( init.nodes[1], &mm_class_of<A>::the() );
    test( init.nodes[2], &mm_class_of<B>::the() );
    test( init.nodes[3], &mm_class_of<C>::the() );
    test( init.nodes[4], &mm_class_of<BC>::the() );
    test( init.nodes[5], &mm_class_of<D>::the() );
    test( init.nodes[6], &mm_class_of<CD>::the() );
    test( init.nodes[7], &mm_class_of<Y>::the() );

    init.make_masks();
    testx( init.nodes[0]->mask, dynamic_bitset<>(8, 0b11111111) ); // X
    testx( init.nodes[1]->mask, dynamic_bitset<>(8, 0b01111110) ); // A
    testx( init.nodes[2]->mask, dynamic_bitset<>(8, 0b00010100) ); // B
    testx( init.nodes[3]->mask, dynamic_bitset<>(8, 0b01011000) ); // C
    testx( init.nodes[4]->mask, dynamic_bitset<>(8, 0b00010000) ); // BC
    testx( init.nodes[5]->mask, dynamic_bitset<>(8, 0b01100000) ); // D
    testx( init.nodes[6]->mask, dynamic_bitset<>(8, 0b01000000) ); // CD
    testx( init.nodes[7]->mask, dynamic_bitset<>(8, 0b10000000) ); // Y

    init.assign_slots();
    test(m_x.the().slots[0], 0);
    test(m_a.the().slots[0], 1);
    test(m_b.the().slots[0], 2);
    test(m_c.the().slots[0], 3);
    test(m_bc.the().slots[0], 4);
    test(m_d.the().slots[0], 2);
    test(m_cd.the().slots[0], 4);
    test(m_y.the().slots[0], 1);

    test(mm_class_of<X>::the().mmt.size(), 1);
    test(mm_class_of<A>::the().mmt.size(), 2);
    test(mm_class_of<B>::the().mmt.size(), 3);
    test(mm_class_of<C>::the().mmt.size(), 4);
    test(mm_class_of<BC>::the().mmt.size(), 5);
    test(mm_class_of<D>::the().mmt.size(), 3);
    test(mm_class_of<CD>::the().mmt.size(), 5);
    test(mm_class_of<Y>::the().mmt.size(), 2);
  }
}
namespace grouping_resolver_tests {

  DO {
    cout << "\n--- Grouping resolver tests\n";
  }
  
  #include "animals.hpp"

#define MAKE_CLASS(Class, Base)                 \
  struct Class : Base {                         \
  MM_CLASS(Class, Base);                        \
  Class() {                                     \
    MM_INIT();                                  \
  }                                             \
  }

  MAKE_CLASS(Mobile, Interface);
  MAKE_CLASS(MSWindows, Window);
  MAKE_CLASS(X, Window);
  MAKE_CLASS(Nokia, Mobile);
  MAKE_CLASS(Samsung, Mobile);

  enum action { whatever, print_herbivore, draw_herbivore, print_carnivore, draw_carnivore, mobile };

  MULTIMETHOD(display, action(const virtual_<Animal>&, const virtual_<Interface>&));

  // 0
  BEGIN_METHOD(display, action, const Herbivore& a, const Terminal& b) {
    return print_herbivore;
  } END_METHOD;

  // 1
  BEGIN_METHOD(display, action, const Herbivore& a, const Window& b) {
    return draw_herbivore;
  } END_METHOD;

  // 2
  BEGIN_METHOD(display, action, const Carnivore& a, const Terminal& b) {
    return print_carnivore;
  } END_METHOD;

  // 3
  BEGIN_METHOD(display, action, const Carnivore& a, const Window& b) {
    return draw_carnivore;
  } END_METHOD;

  // 4
  BEGIN_METHOD(display, action, const Animal& a, const Mobile& b) {
    return mobile;
  } END_METHOD;

  // we want to build:
  //              Interface   Terminal   Window+ Mobile+
  // Animal       0           0          0       mob
  // Herbivore+   0           p_herb     d_herb  mob
  // Carnivore+   0           p_carn     d_carn  mob

  DO {
    hierarchy_initializer::initialize(mm_class_of<Animal>::the());
    hierarchy_initializer::initialize(mm_class_of<Interface>::the());

    grouping_resolver rdisp(display.the());
    
    methods animal_applicable;
    rdisp.find_applicable(0, &mm_class_of<Animal>::the(), animal_applicable);
    test( animal_applicable, methods { display.the().methods[4] } );

    methods herbivore_applicable;
    rdisp.find_applicable(0, &mm_class_of<Herbivore>::the(), herbivore_applicable);
    test( herbivore_applicable, (methods { display.the().methods[0], display.the().methods[1], display.the().methods[4] } ));

    methods cow_applicable;
    rdisp.find_applicable(0, &mm_class_of<Cow>::the(), cow_applicable);
    test( cow_applicable, (methods { display.the().methods[0], display.the().methods[1], display.the().methods[4] } ));

    methods carnivore_applicable;
    rdisp.find_applicable(0, &mm_class_of<Carnivore>::the(), carnivore_applicable);
    test( carnivore_applicable, (methods { display.the().methods[2],  display.the().methods[3], display.the().methods[4] }) );

    methods wolf_applicable;
    rdisp.find_applicable(0, &mm_class_of<Wolf>::the(), wolf_applicable);
    test( wolf_applicable, (methods { display.the().methods[2],  display.the().methods[3], display.the().methods[4] }) );

    methods interface_applicable;
    rdisp.find_applicable(1, &mm_class_of<Interface>::the(), interface_applicable);
    test( interface_applicable, methods { } );

    methods terminal_applicable;
    rdisp.find_applicable(1, &mm_class_of<Terminal>::the(), terminal_applicable);
    test( terminal_applicable, (methods { display.the().methods[0], display.the().methods[2] }) );

    methods window_applicable;
    rdisp.find_applicable(1, &mm_class_of<Window>::the(), window_applicable);
    test( window_applicable, (methods { display.the().methods[1], display.the().methods[3] }) );

    methods mobile_applicable;
    rdisp.find_applicable(1, &mm_class_of<Mobile>::the(), mobile_applicable);
    test( mobile_applicable, (methods { display.the().methods[4] }) );

// Animal = class_0
// Herbivore = class_1
// Cow = class_2
// Carnivore = class_3
// Wolf = class_4
// Tiger = class_5
    
// Interface = class_0
// Terminal = class_1
// Window = class_2
// MsWindows = class_3
// X = class_4
// Mobile = class_5
// Nokia = class_6
// Samsung = class_7
    
    rdisp.make_groups();
    test( rdisp.groups.size(), 2 ) &&
      test( rdisp.groups[0].size(), 3) &&
      test( rdisp.groups[0][0].methods, animal_applicable) &&
      test( rdisp.groups[0][1].methods, herbivore_applicable) &&
      test( rdisp.groups[0][2].methods, carnivore_applicable) &&
      test( rdisp.groups[1].size(), 4) &&
      test( rdisp.groups[1][0].methods, interface_applicable) &&
      test( rdisp.groups[1][1].methods, terminal_applicable) &&
      test( rdisp.groups[1][2].methods, window_applicable) &&
      test( rdisp.groups[1][3].methods, mobile_applicable);

    test(display.the().steps.size(), 2) &&
      test(display.the().steps[0], 1) &&
      test(display.the().steps[1], 3);

    test( (*Animal().__mm_ptbl)[0], 0 );
    test( (*Herbivore().__mm_ptbl)[0], 1 );
    test( (*Cow().__mm_ptbl)[0], 1 );
    test( (*Carnivore().__mm_ptbl)[0], 2 );
    test( (*Wolf().__mm_ptbl)[0], 2 );
    test( (*Tiger().__mm_ptbl)[0], 2 );
    test( (*Interface().__mm_ptbl)[1], 0 );
    test( (*Terminal().__mm_ptbl)[0], 1 );
    test( (*Window().__mm_ptbl)[0], 2 );
    test( (*MSWindows().__mm_ptbl)[0], 2 );
    test( (*X().__mm_ptbl)[0], 2 );
    test( (*Mobile().__mm_ptbl)[0], 3 );
    test( (*Nokia().__mm_ptbl)[0], 3 );
    test( (*Samsung().__mm_ptbl)[0], 3 );

    rdisp.make_table();
    auto table = display.the().dispatch_table;
    auto methods = display.the().methods;
    using method = decltype(display)::method_entry;
    
                                                                                        
    test( table != 0, true);
      // Interface
    test( table[0], throw_undefined<decltype(display)::signature>::body );
    test( table[1], throw_undefined<decltype(display)::signature>::body );
    test( table[2], throw_undefined<decltype(display)::signature>::body );

      // Terminal
    test( table[3], throw_undefined<decltype(display)::signature>::body );
    test( table[4], static_cast<method*>(methods[0])->pm );
    test( table[5], static_cast<method*>(methods[2])->pm );

      // Window
    test( table[6], throw_undefined<decltype(display)::signature>::body );
    test( table[7], static_cast<method*>(methods[1])->pm );
    test( table[8], static_cast<method*>(methods[3])->pm );

    // Mobile
    test( table[9], static_cast<method*>(methods[4])->pm );
    test( table[10], static_cast<method*>(methods[4])->pm );
    test( table[11], static_cast<method*>(methods[4])->pm );
    
    rdisp.assign_next();

    test( display(Herbivore(), Terminal()), print_herbivore );
    test( display(Cow(), Terminal()), print_herbivore );

    test( display(Carnivore(), Terminal()), print_carnivore );
    test( display(Wolf(), Terminal()), print_carnivore );
    test( display(Tiger(), Terminal()), print_carnivore );

    test( display(Herbivore(), Window()), draw_herbivore );
    test( display(Cow(), Window()), draw_herbivore );
    test( display(Cow(), MSWindows()), draw_herbivore );
    test( display(Cow(), X()), draw_herbivore );

    test( display(Carnivore(), Window()), draw_carnivore );
    test( display(Wolf(), X()), draw_carnivore );
    test( display(Tiger(), MSWindows()), draw_carnivore );

    test( display(Herbivore(), Samsung()), mobile );
    test( display(Cow(), Nokia()), mobile );
    
    test( display(Carnivore(), Samsung()), mobile );
    test( display(Wolf(), Nokia()), mobile );
  }
  
}

namespace init_tests {

#include "animals.hpp"

  MULTIMETHOD(encounter, string(const virtual_<Animal>&, const virtual_<Animal>&));

  BEGIN_METHOD(encounter, string, const Animal&, const Animal&) {
    return "ignore";
  } END_METHOD;

  DO {
    multimethods::initialize();
    test(encounter(Cow(), Wolf()), "ignore");
    test(encounter(Wolf(), Cow()), "ignore");
  }

  BEGIN_METHOD(encounter, string, const Herbivore&, const Carnivore&) {
    return "run";
  } END_METHOD;

  DO {
    multimethods::initialize();
    test(encounter(Cow(), Wolf()), "run");
    test(encounter(Wolf(), Cow()), "ignore");
  }

  BEGIN_METHOD(encounter, string, const Carnivore&, const Herbivore&) {
    return "hunt";
  } END_METHOD;

  DO {
    multimethods::initialize();
    test(encounter(Cow(), Wolf()), "run");
    test(encounter(Wolf(), Cow()), "hunt");
  }

  struct Horse : Herbivore {
    MM_CLASS(Horse, Herbivore);
    Horse() {
      MM_INIT();
    }
  };

  DO {
    multimethods::initialize();
    test(encounter(Horse(), Wolf()), "run");
    test(encounter(Wolf(), Horse()), "hunt");
  }
}

namespace single_inheritance {

  DO {
    cout << "\n--- Single inheritance." << endl;
  }

  MULTIMETHOD(encounter, string(virtual_<Animal>&, virtual_<Animal>&));
  
  BEGIN_METHOD(encounter, string, Animal&, Animal&) {
    return "ignore";
  } END_METHOD;

  BEGIN_METHOD(encounter, string, Carnivore&, Animal&) {
    return "hunt";
  } END_METHOD;

  BEGIN_METHOD(encounter, string, Carnivore&, Carnivore&) {
    return "fight";
  } END_METHOD;

  BEGIN_METHOD(encounter, string, Wolf&, Wolf&) {
    return "wag tail";
  } END_METHOD;

  BEGIN_METHOD(encounter, string, Herbivore&, Carnivore&) {
    return "run";
  } END_METHOD;

  enum action { display_error, print_cow, draw_cow, print_wolf, draw_wolf, print_tiger, draw_tiger, print_herbivore, display_cow, print_animal };

  MULTIMETHOD(display, action(virtual_<Animal>&, virtual_<Interface>&));

  BEGIN_METHOD(display, action, Cow& a, Terminal& b) {
    return print_cow;
  } END_METHOD;

  BEGIN_METHOD(display, action, Wolf& a, Terminal& b) {
    return print_wolf;
  } END_METHOD;

  BEGIN_METHOD(display, action, Tiger& a, Terminal& b) {
    return print_tiger;
  } END_METHOD;

  BEGIN_METHOD(display, action, Cow& a, Window& b) {
    return draw_cow;
  } END_METHOD;

  BEGIN_METHOD(display, action, Wolf& a, Window& b) {
    return draw_wolf;
  } END_METHOD;

  BEGIN_METHOD(display, action, Tiger& a, Window& b) {
    return draw_tiger;
  } END_METHOD;

// following two are ambiguous, e.g. for (Cow, Terminal)

  BEGIN_METHOD(display, action, Herbivore& a, Interface& b) {
    return display_error;
  } END_METHOD;

  BEGIN_METHOD(display, action, Animal& a, Terminal& b) {
    return display_error;
  } END_METHOD;

  DO {
    Cow c;
    Wolf w;
    Tiger t;
    Terminal term;
    Window win;
    Interface interf;
    Herbivore herb;

    multimethods::initialize();
    
    test(encounter(c, w), "run");
    test(encounter(c, c), "ignore");

    // static call
    test(STATIC_CALL_METHOD(encounter, string(Animal&, Animal&))(c, w), "ignore");
      
    // next
    test(encounter_method<string(Wolf&, Wolf&)>::next(w, w), "fight");
    test(encounter_method<string(Carnivore&, Carnivore&)>::next(w, w), "hunt");
    test(encounter_method<string(Carnivore&, Animal&)>::next(w, w), "ignore");
  }
}

namespace mi {
  #include "mi.hpp"

  MULTIMETHOD(encounter, string(virtual_<Animal>&, virtual_<Animal>&));
  
  BEGIN_METHOD(encounter, string, Animal&, Animal&) {
    return "ignore";
  } END_METHOD;

  BEGIN_METHOD(encounter, string, Stallion&, Mare&) {
    return "court";
  } END_METHOD;

  BEGIN_METHOD(encounter, string, Predator&, Herbivore&) {
    return "hunt";
  } END_METHOD;

  DO {
    Stallion stallion;
    Mare mare;
    Wolf wolf;

    static_assert(is_virtual_base_of<Animal, Stallion>::value, "problem with virtual base detection");

    multimethods::initialize();
    
    test( encounter(stallion, mare), "court" );
    test( encounter(mare, mare), "ignore" );
    test( encounter(wolf, mare), "hunt" );
  }
}

int main() {
    cout << "\n" << success << " tests succeeded, " << failure << " failed.\n";
  return 0;
}

