/*
 * Copyright (c) 2011 Matt Fichman
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, APEXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */  

#include "Feature.hpp"
#include "Environment.hpp"
#include <iostream>

using namespace std;

Class::Class(Location loc, Environment* env, Type* type, Type* mixins, 
    String* comment, Feature* feat) :

    Feature(loc, env, type->name()),
    type_(type),
    mixins_(mixins),
    comment_(comment),
    features_(feat),
    is_object_(false),
    is_value_(false),
    is_interface_(false),
    is_mixin_(false),
    is_enum_(false),
    size_(0) {

    for (Feature* feat = features_; feat; feat = feat->next()) {
        feat->parent(this);
        if (!feature_[feat->name()]) {
            feature_[feat->name()] = feat;
        }        
    }

    for (Type* mixin = mixins; mixin; mixin = mixin->next()) {
        if ("Object" == mixin->name()->string()) {
            is_object_ = true;
            break;
        }
        if ("Interface" == mixin->name()->string()) {
            is_interface_ = true;
            break;
        }
        if ("Value" == mixin->name()->string()) {
            is_value_ = true;
            break;
        }
        if ("Mixin" == mixin->name()->string()) {
            is_mixin_ = true;
            break;
        }
        if ("Enum" == mixin->name()->string()) {
            is_enum_ = true;
            is_value_ = true;
            gen_equal_method();
            break;
        }
    }
}

/* Constructor for union types */
Class::Class(Location loc, Environment* env, Type* type, Type* alt) :
    Feature(loc, env, type->name()),
    type_(type),
    alternates_(alt),
    is_object_(true),
    is_value_(false),
    is_interface_(false),
    is_mixin_(false), 
    is_enum_(false),
    size_(0) {

}

/* Constructor for enum types */
Class::Class(Location loc, Environment* env, Type* type, Feature* feat) :
    Feature(loc, env, type->name()),
    type_(type),
    features_(feat),
    is_object_(false),
    is_value_(true),
    is_interface_(false),
    is_mixin_(false), 
    is_enum_(true),
    size_(0) {

    for (Feature* feat = features_; feat; feat = feat->next()) {
        feat->parent(this);
        if (!feature_[feat->name()]) {
            feature_[feat->name()] = feat;
        }        
    }
    gen_equal_method();
}

void Class::gen_equal_method() {
    // Generates an automatic @equal method when necessary.
    String::Ptr name = env()->name("@equal");
    if(!feature(name)) {
        Type::Ptr ret = env()->bool_type();
        Formal::Ptr self(new Formal(location(), env()->name("self"), type())); 
        self->next(new Formal(location(), env()->name("other"), type())); 
        Feature::Flags flags = Feature::NATIVE;
        feature(new Function(location(), env(), name, self, flags, ret, 0));
    }
}

String* Class::default_enum_value() const {
    // Returns the first (default) value in the enumeration.
    for (Feature* feat = features_; feat; feat = feat->next()) {
        if (Constant* cons = dynamic_cast<Constant*>(feat)) {
            Expression* init = cons->initializer();
            if (IntegerLiteral* lit = dynamic_cast<IntegerLiteral*>(init)) {
                return lit->value();
            }
        }
    }
    return env()->integer("0");
}

void Class::jump1(int index, int d) {
    if (index >= jump1_.size()) {
        jump1_.resize(index+1);
    }
    jump1_[index] = d; 
}

void Class::jump2(int index, Function* func) {
    if (index >= jump2_.size()) {
        jump2_.resize(index+1);
    }
    jump2_[index] = func; 
}

void Class::feature(Feature* feature) {
    if (!feature) {
        return;
    }

    feature->parent(this);
    features_ = append(features_, feature);
    if (!feature_[feature->name()]) {
        feature_[feature->name()] = feature;
    }
}

Feature* Class::feature(String* name) const {
    // Searches for a feature with name 'name' in this lass or in a mixin
    // that was added to this class.

    std::map<String::Ptr, Feature::Ptr>::const_iterator i = feature_.find(name);
    if (i != feature_.end()) {
        return i->second;
    }

    if (type()->is_proto()) {
        return 0;
    }

    for (Type* mixin = mixins_; mixin; mixin = mixin->next()) {
        Class* clazz = mixin->clazz();
        if (clazz) {
            Feature* func = clazz->feature(name);
            if (func) {
                return func;
            } 
        }
    }
    return 0;
}

bool Class::subtype(Class* other) const {
    // Returns true if this class is a subtype of 'other.'  A class is a subtype
    // of another class if it implements all methods found in the class.

    // Check if this comparison has been done before.  If so, then we don't need
    // to do it again.
    std::map<Class*, bool>::iterator i = subtype_.find(other);
    if (i != subtype_.end()) {
        return i->second;
    }

    // If 'this' is a union type, then it will be a subtype of 'other' if
    // 'other' is listed as an alternate.  Likewise, 'this' will be a subtype
    // of 'other' if 'other' is an alternate and 'this' is listed as an
    // alternate of 'other'.
    if (alternates()) {
        for (Type::Ptr alt = alternates(); alt; alt = alt->next()) {
            if (alt->clazz() == other) {
                return true;
            }  
        }
        return false;
    } else if (other->alternates()) {
        for (Type::Ptr alt = other->alternates(); alt; alt = alt->next()) {
            if (alt->clazz() == this) {
                return true;
            }
        }
        return false;
    } else if (!other->is_interface()) {
        return this == other; 
    }

    // Assume that this type is a subtype of 'other.'  This will break cycles
    // caused when checking function return values for covariance.
    subtype_[other] = true;

    for (Feature* f = other->features(); f; f = f->next()) {
        if (Function* func = dynamic_cast<Function*>(f)) {
            Function* mine = function(func->name());   
            if (!mine || !mine->covariant(func)) {
                // Now we know that the types are not compatible, so mark
                // 'other' as a disjoint type.
                subtype_[other] = false;
                return false;
            }
        }
    }
    return true;
}

Constant::Constant(Location loc, Environment* env, String* name, Flags flags, 
    Expression* init) :

    Feature(loc, env, name, flags),
    initializer_(init) {
}

Function::Function(Location loc, Environment* env, String* name, Formal* formal, 
    Flags flags, Type* ret, Block* block) :

    Feature(loc, env, name, flags),
	formals_(formal),
	type_(ret),
    block_(block),
    stack_vars_(0),
	throw_spec_(UNKNOWN) {
}

bool Function::covariant(Function* other) const {
    // Returns true if this function is covariant, i.e., its return type is the
    // same as or a subtype of 'other.'  The arguments must match exactly.
    Formal* f1 = formals_;
    Formal* f2 = other->formals_;

    while (f1 && f2) {
        if (f1->type()->name()->string() == "Self" 
            && f2->type()->name()->string() == "Self") {
            /* pass */
        } else if (!f1->type()->equals(f2->type())) {
            return false;
        }
        f1 = f1->next();
        f2 = f2->next();
    }
    if (f1 || f2) {
        return false;
    }
    if (!type_->subtype(other->type())) {
        return false;
    } 

    return true;
}

bool Function::is_primitive_op() const {
	// Returns true if this function is a primitive operand, which is output
	// directly into machine code.
	Class* clazz = dynamic_cast<Class*>(parent());
	return clazz && clazz->is_primitive() && name()->string()[0] == '@';
}

bool Function::is_virtual() const {
    // Returns true if the function is virtual.
    Class* clazz = dynamic_cast<Class*>(parent());
    return clazz && clazz->is_interface();
}

Function::ThrowSpec Function::throw_spec() const {
	// Returns THROW if this function can throw an exception; otherwise
	// returns NOTHROW.  The result is computed lazily, but all of the 
	// children must be added to the called_func array or the computation
	// will be reset.
	if (throw_spec_ != UNKNOWN) {
		return throw_spec_;
	}
	if (is_virtual()) {
		throw_spec_ = THROW;
		return throw_spec_;
	}
    throw_spec_ = NOTHROW;
	for (int i = 0; i < called_func_.size(); i++) {
		if (called_func_[i]->throw_spec() == THROW) {
			throw_spec_ = THROW;
			return throw_spec_;
		}
	}
	
	return throw_spec_;
}

void Function::called_func(Function* func) {
	// Reset the throw_spec, because adding a new function that throws
	// could cause this function to throw.
	called_func_.push_back(func);
}

Module::Module(Location loc, Environment* env, String* name) :
    Feature(loc, env, name),
    is_input_(env->is_input(name->string())) {
    if (name->string() == "Boot") {
        is_input_ = false;
    }
}

void Module::feature(Feature* feature) {
    if (!feature) {
        return;
    }

    feature->parent(this);
    features_ = append(features_, feature);
    if (!feature_[feature->name()]) {
        feature_[feature->name()] = feature;
    }
}

void Module::import(Import* import) {
    if (!import) {
        return; 
    }

    Import::Ptr im(import);
    if (!import_[import->scope()]) {
        import_[import->scope()] = import;
        imports_ = append(imports_, import);
    }
}

bool Module::is_up_to_date() const {
    // Return true if the module is up to date, that is, if all dependent
    // source files have a later modification time than the output file.
    std::string out = function(env()->name("main")) ? lib_file() : exe_file();
    for (int i = 0; i < files(); i++) {
        if (!File::is_up_to_date(file(i)->path()->string(), out)) {
            return false;
        }
    }
    return true;
}

std::string Module::lib_file() const {
    std::string dir = env()->output() + FILE_SEPARATOR + "lib";
#ifdef WINDOWS
    return dir + FILE_SEPARATOR + name()->string() + ".lib";
#else
    return dir + FILE_SEPARATOR + "lib" + name()->string() + ".a";
#endif
}

std::string Module::exe_file() const {
    std::string dir = env()->output() + FILE_SEPARATOR + "bin";
#ifdef WINDOWS
    return dir + FILE_SEPARATOR + name()->string() + ".exe";
#else
    return dir + FILE_SEPARATOR + name()->string();
#endif
} 

Class* Feature::clazz(String* name) const {
    return dynamic_cast<Class*>(feature(name));
}

Attribute* Feature::attribute(String* name) const {
    return dynamic_cast<Attribute*>(feature(name));
}

Constant* Feature::constant(String* name) const {
    return dynamic_cast<Constant*>(feature(name));
}

Function* Feature::function(String* name) const {
    return dynamic_cast<Function*>(feature(name));
}

String* Feature::qualified_name() const {
    // Returns the qualified name for this feature

    std::string parent = parent_ ? parent_->qualified_name()->string() : "";
    std::string name = parent.empty() ? "" : parent + "::";
    name += Feature::name()->string(); 
    return env_->name(name);
}

String* Feature::label() const {
    // Builds the label for a feature if it hasn't already been calculated.
    if (label_) { return label_; }

    std::string parent = parent_ ? parent_->label()->string() : "";
    std::string name = parent.empty() ? "" : parent + "_";
    name += Feature::name()->string();    

    std::string label;
    label.reserve(name.length());
    for (size_t i = 0; i < name.length(); i++) {
        if (name[i] == '@') {
            label += '_';
        } else if (name[i] == '?') {
            label += "__g";
        } else if (name[i] == '!') {
            label += "__w";
        } else if (name[i] == '=') {
            label += "__s";
        } else if (name[i] == ':') {
            label += "_";
            i++;
        } else {
            label += name[i];
        }
    }

    label_ = env_->name(label);
    return label_;
}

