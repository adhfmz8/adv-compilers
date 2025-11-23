; ModuleID = 'alias_test.ll'
source_filename = "alias_test.ll"
target datalayout = "e-m:o-i64:64-i128:128-n32:64-S128"
target triple = "arm64-apple-macosx14.0.0"

; ---------------------------------------------------------
; TEST 1: Must Alias
; Both stores write to %p. The address is identical.
; EXPECTED: Remove 'store i32 10'
; ---------------------------------------------------------
define void @test_must_alias(ptr %p) {
  store i32 10, ptr %p, align 4
  store i32 20, ptr %p, align 4
  ret void
}

; ---------------------------------------------------------
; TEST 2: May Alias
; %p and %q are separate arguments. They MIGHT point to the
; same address, or they might not.
; Since LLVM can't prove they are the same, it must assume
; they might be different. Therefore, the first store is
; NOT safe to remove.
; EXPECTED: No output
; ---------------------------------------------------------
define void @test_may_alias(ptr %p, ptr %q) {
  store i32 10, ptr %p, align 4
  store i32 20, ptr %q, align 4
  ret void
}

; ---------------------------------------------------------
; TEST 3: Must Alias but Intervening Load
; Both stores write to %p. However, the value is read
; in between. Removing the first store would break the program.
; EXPECTED: No output
; ---------------------------------------------------------
define void @test_intervening_load(ptr %p) {
  store i32 10, ptr %p, align 4
  %val = load i32, ptr %p, align 4  ; <--- Uses the 10
  store i32 20, ptr %p, align 4
  ret void
}

; ---------------------------------------------------------
; TEST 4: Must Alias (GEP Math)
; LLVM BasicAA is smart enough to know that %p + 0
; is the same as %p.
; EXPECTED: Remove 'store i32 30'
; ---------------------------------------------------------
define void @test_gep_alias(ptr %p) {
  %ptr_b = getelementptr i32, ptr %p, i32 0
  store i32 30, ptr %p, align 4
  store i32 40, ptr %ptr_b, align 4
  ret void
}