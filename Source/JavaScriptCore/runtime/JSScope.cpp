/*
 * Copyright (C) 2012-2015 Apple Inc. All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#include "config.h"
#include "JSScope.h"

#include "JSGlobalObject.h"
#include "JSLexicalEnvironment.h"
#include "JSNameScope.h"
#include "JSWithScope.h"
#include "JSCInlines.h"

namespace JSC {

STATIC_ASSERT_IS_TRIVIALLY_DESTRUCTIBLE(JSScope);

void JSScope::visitChildren(JSCell* cell, SlotVisitor& visitor)
{
    JSScope* thisObject = jsCast<JSScope*>(cell);
    ASSERT_GC_OBJECT_INHERITS(thisObject, info());
    Base::visitChildren(thisObject, visitor);
    visitor.append(&thisObject->m_next);
}

// Returns true if we found enough information to terminate optimization.
static inline bool abstractAccess(ExecState* exec, JSScope* scope, const Identifier& ident, GetOrPut getOrPut, size_t depth, bool& needsVarInjectionChecks, ResolveOp& op)
{
    if (JSLexicalEnvironment* lexicalEnvironment = jsDynamicCast<JSLexicalEnvironment*>(scope)) {
        if (ident == exec->propertyNames().arguments) {
            // We know the property will be at this lexical environment scope, but we don't know how to cache it.
            op = ResolveOp(Dynamic, 0, 0, 0, 0, 0);
            return true;
        }

        SymbolTableEntry entry = lexicalEnvironment->symbolTable()->get(ident.impl());
        if (entry.isReadOnly() && getOrPut == Put) {
            // We know the property will be at this lexical environment scope, but we don't know how to cache it.
            op = ResolveOp(Dynamic, 0, 0, 0, 0, 0);
            return true;
        }

        if (!entry.isNull()) {
            op = ResolveOp(makeType(ClosureVar, needsVarInjectionChecks), depth, 0, lexicalEnvironment, entry.watchpointSet(), entry.scopeOffset().offset());
            return true;
        }

        if (lexicalEnvironment->symbolTable()->usesNonStrictEval())
            needsVarInjectionChecks = true;
        return false;
    }

    if (JSGlobalObject* globalObject = jsDynamicCast<JSGlobalObject*>(scope)) {
        SymbolTableEntry entry = globalObject->symbolTable()->get(ident.impl());
        if (!entry.isNull()) {
            if (getOrPut == Put && entry.isReadOnly()) {
                // We know the property will be at global scope, but we don't know how to cache it.
                op = ResolveOp(Dynamic, 0, 0, 0, 0, 0);
                return true;
            }

            op = ResolveOp(
                makeType(GlobalVar, needsVarInjectionChecks), depth, 0, 0, entry.watchpointSet(),
                reinterpret_cast<uintptr_t>(globalObject->variableAt(entry.scopeOffset()).slot()));
            return true;
        }

        PropertySlot slot(globalObject);
        if (!globalObject->getOwnPropertySlot(globalObject, exec, ident, slot)
            || !slot.isCacheableValue()
            || !globalObject->structure()->propertyAccessesAreCacheable()
            || (globalObject->structure()->hasReadOnlyOrGetterSetterPropertiesExcludingProto() && getOrPut == Put)) {
            // We know the property will be at global scope, but we don't know how to cache it.
            ASSERT(!scope->next());
            op = ResolveOp(makeType(GlobalProperty, needsVarInjectionChecks), depth, 0, 0, 0, 0);
            return true;
        }
        
        WatchpointState state = globalObject->structure()->ensurePropertyReplacementWatchpointSet(exec->vm(), slot.cachedOffset())->state();
        if (state == IsWatched && getOrPut == Put) {
            // The field exists, but because the replacement watchpoint is still intact. This is
            // kind of dangerous. We have two options:
            // 1) Invalidate the watchpoint set. That would work, but it's possible that this code
            //    path never executes - in which case this would be unwise.
            // 2) Have the invalidation happen at run-time. All we have to do is leave the code
            //    uncached. The only downside is slightly more work when this does execute.
            // We go with option (2) here because it seems less evil.
            op = ResolveOp(makeType(GlobalProperty, needsVarInjectionChecks), depth, 0, 0, 0, 0);
        } else
            op = ResolveOp(makeType(GlobalProperty, needsVarInjectionChecks), depth, globalObject->structure(), 0, 0, slot.cachedOffset());
        return true;
    }

    op = ResolveOp(Dynamic, 0, 0, 0, 0, 0);
    return true;
}

JSObject* JSScope::objectAtScope(JSScope* scope)
{
    JSObject* object = scope;
    if (object->type() == WithScopeType)
        return jsCast<JSWithScope*>(object)->object();

    return object;
}

int JSScope::depth()
{
    int depth = 0;
    for (JSScope* scope = this; scope; scope = scope->next())
        ++depth;
    return depth;
}

// When an exception occurs, the result of isUnscopable becomes false.
static inline bool isUnscopable(ExecState* exec, JSScope* scope, JSObject* object, const Identifier& ident)
{
    if (scope->type() != WithScopeType)
        return false;

    JSValue unscopables = object->get(exec, exec->propertyNames().unscopablesSymbol);
    if (exec->hadException())
        return false;
    if (!unscopables.isObject())
        return false;
    JSValue blocked = jsCast<JSObject*>(unscopables)->get(exec, ident);
    if (exec->hadException())
        return false;

    return blocked.toBoolean(exec);
}

JSValue JSScope::resolve(ExecState* exec, JSScope* scope, const Identifier& ident)
{
    ScopeChainIterator end = scope->end();
    ScopeChainIterator it = scope->begin();
    while (1) {
        JSScope* scope = it.scope();
        JSObject* object = it.get();

        if (++it == end) // Global scope.
            return object;

        if (object->hasProperty(exec, ident)) {
            if (!isUnscopable(exec, scope, object, ident))
                return object;
            ASSERT_WITH_MESSAGE(!exec->hadException(), "When an exception occurs, the result of isUnscopable becomes false");
        }
    }
}

ResolveOp JSScope::abstractResolve(ExecState* exec, size_t depthOffset, JSScope* scope, const Identifier& ident, GetOrPut getOrPut, ResolveType unlinkedType)
{
    ResolveOp op(Dynamic, 0, 0, 0, 0, 0);
    if (unlinkedType == Dynamic)
        return op;

    bool needsVarInjectionChecks = JSC::needsVarInjectionChecks(unlinkedType);
    size_t depth = depthOffset;
    for (; scope; scope = scope->next()) {
        if (abstractAccess(exec, scope, ident, getOrPut, depth, needsVarInjectionChecks, op))
            break;
        ++depth;
    }

    return op;
}

void JSScope::collectVariablesUnderTDZ(JSScope* scope, VariableEnvironment& result)
{
    for (; scope; scope = scope->next()) {
        if (!scope->isLexicalScope())
            continue;
        SymbolTable* symbolTable = jsCast<JSLexicalEnvironment*>(scope)->symbolTable();
        ASSERT(symbolTable->correspondsToLexicalScope());
        ConcurrentJITLocker locker(symbolTable->m_lock);
        for (auto end = symbolTable->end(locker), iter = symbolTable->begin(locker); iter != end; ++iter)
            result.add(iter->key);
    }
}

bool JSScope::isLexicalScope()
{
    JSLexicalEnvironment* lexicalEnvironment = jsDynamicCast<JSLexicalEnvironment*>(this);
    if (!lexicalEnvironment) // Global object does not hold any lexical variables so we can ignore it.
        return false;

    return lexicalEnvironment->symbolTable()->correspondsToLexicalScope();
}

const char* resolveModeName(ResolveMode mode)
{
    static const char* const names[] = {
        "ThrowIfNotFound",
        "DoNotThrowIfNotFound"
    };
    return names[mode];
}

const char* resolveTypeName(ResolveType type)
{
    static const char* const names[] = {
        "GlobalProperty",
        "GlobalVar",
        "ClosureVar",
        "LocalClosureVar",
        "GlobalPropertyWithVarInjectionChecks",
        "GlobalVarWithVarInjectionChecks",
        "ClosureVarWithVarInjectionChecks",
        "Dynamic"
    };
    return names[type];
}

} // namespace JSC
