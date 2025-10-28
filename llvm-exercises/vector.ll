; ModuleID = 'vector.c'
source_filename = "vector.c"
target datalayout = "e-m:o-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-n32:64-S128-Fn32"
target triple = "arm64-apple-macosx14.0.0"

@__const.main.x = private unnamed_addr constant [3 x double] [double 1.000000e+00, double 2.000000e+00, double 3.000000e+00], align 8
@__const.main.y = private unnamed_addr constant [3 x double] [double 4.000000e+00, double 5.000000e+00, double 6.000000e+00], align 8

; Function Attrs: nounwind ssp uwtable(sync)
define double @cosine_similarity(ptr noundef %0, ptr noundef %1, i64 noundef %2) #0 {
  %4 = alloca ptr, align 8
  %5 = alloca ptr, align 8
  %6 = alloca i64, align 8
  %7 = alloca double, align 8
  %8 = alloca double, align 8
  %9 = alloca double, align 8
  store ptr %0, ptr %4, align 8
  store ptr %1, ptr %5, align 8
  store i64 %2, ptr %6, align 8
  %10 = load ptr, ptr %4, align 8
  %11 = load ptr, ptr %5, align 8
  %12 = load i64, ptr %6, align 8
  %13 = call double @dot(ptr noundef %10, ptr noundef %11, i64 noundef %12)
  store double %13, ptr %7, align 8
  %14 = load ptr, ptr %4, align 8
  %15 = load i64, ptr %6, align 8
  %16 = call double @norm(ptr noundef %14, i64 noundef %15)
  store double %16, ptr %8, align 8
  %17 = load ptr, ptr %5, align 8
  %18 = load i64, ptr %6, align 8
  %19 = call double @norm(ptr noundef %17, i64 noundef %18)
  store double %19, ptr %9, align 8
  %20 = load double, ptr %7, align 8
  %21 = load double, ptr %8, align 8
  %22 = load double, ptr %9, align 8
  %23 = fmul double %21, %22
  %24 = fdiv double %20, %23
  ret double %24
}

; Function Attrs: nounwind ssp uwtable(sync)
define internal double @dot(ptr noundef %0, ptr noundef %1, i64 noundef %2) #0 {
  %4 = alloca ptr, align 8
  %5 = alloca ptr, align 8
  %6 = alloca i64, align 8
  %7 = alloca double, align 8
  %8 = alloca i64, align 8
  store ptr %0, ptr %4, align 8
  store ptr %1, ptr %5, align 8
  store i64 %2, ptr %6, align 8
  store double 0.000000e+00, ptr %7, align 8
  store i64 0, ptr %8, align 8
  br label %9

9:                                                ; preds = %24, %3
  %10 = load i64, ptr %8, align 8
  %11 = load i64, ptr %6, align 8
  %12 = icmp ult i64 %10, %11
  br i1 %12, label %13, label %27

13:                                               ; preds = %9
  %14 = load ptr, ptr %4, align 8
  %15 = load i64, ptr %8, align 8
  %16 = getelementptr inbounds nuw double, ptr %14, i64 %15
  %17 = load double, ptr %16, align 8
  %18 = load ptr, ptr %5, align 8
  %19 = load i64, ptr %8, align 8
  %20 = getelementptr inbounds nuw double, ptr %18, i64 %19
  %21 = load double, ptr %20, align 8
  %22 = load double, ptr %7, align 8
  %23 = call double @llvm.fmuladd.f64(double %17, double %21, double %22)
  store double %23, ptr %7, align 8
  br label %24

24:                                               ; preds = %13
  %25 = load i64, ptr %8, align 8
  %26 = add i64 %25, 1
  store i64 %26, ptr %8, align 8
  br label %9, !llvm.loop !6

27:                                               ; preds = %9
  %28 = load double, ptr %7, align 8
  ret double %28
}

; Function Attrs: nounwind ssp uwtable(sync)
define internal double @norm(ptr noundef %0, i64 noundef %1) #0 {
  %3 = alloca ptr, align 8
  %4 = alloca i64, align 8
  store ptr %0, ptr %3, align 8
  store i64 %1, ptr %4, align 8
  %5 = load ptr, ptr %3, align 8
  %6 = load ptr, ptr %3, align 8
  %7 = load i64, ptr %4, align 8
  %8 = call double @dot(ptr noundef %5, ptr noundef %6, i64 noundef %7)
  %9 = call double @llvm.sqrt.f64(double %8)
  ret double %9
}

; Function Attrs: nounwind ssp uwtable(sync)
define i32 @main(i32 noundef %0, ptr noundef %1) #0 {
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca ptr, align 8
  %6 = alloca [3 x double], align 8
  %7 = alloca [3 x double], align 8
  store i32 0, ptr %3, align 4
  store i32 %0, ptr %4, align 4
  store ptr %1, ptr %5, align 8
  call void @llvm.memcpy.p0.p0.i64(ptr align 8 %6, ptr align 8 @__const.main.x, i64 24, i1 false)
  call void @llvm.memcpy.p0.p0.i64(ptr align 8 %7, ptr align 8 @__const.main.y, i64 24, i1 false)
  %8 = getelementptr inbounds [3 x double], ptr %6, i64 0, i64 0
  %9 = getelementptr inbounds [3 x double], ptr %7, i64 0, i64 0
  %10 = call double @cosine_similarity(ptr noundef %8, ptr noundef %9, i64 noundef 3)
  %11 = fmul double %10, 1.000000e+03
  %12 = fptosi double %11 to i32
  ret i32 %12
}

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias writeonly captures(none), ptr noalias readonly captures(none), i64, i1 immarg) #1

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare double @llvm.fmuladd.f64(double, double, double) #2

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare double @llvm.sqrt.f64(double) #2

attributes #0 = { noinline nounwind ssp uwtable(sync) "frame-pointer"="non-leaf" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="apple-m1" "target-features"="+aes,+altnzcv,+ccdp,+ccidx,+ccpp,+complxnum,+crc,+dit,+dotprod,+flagm,+fp-armv8,+fp16fml,+fptoint,+fullfp16,+jsconv,+lse,+neon,+pauth,+perfmon,+predres,+ras,+rcpc,+rdm,+sb,+sha2,+sha3,+specrestrict,+ssbs,+v8.1a,+v8.2a,+v8.3a,+v8.4a,+v8a" }
attributes #1 = { nocallback nofree nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}

!0 = !{i32 2, !"SDK Version", [2 x i32] [i32 14, i32 4]}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 8, !"PIC Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 1}
!4 = !{i32 7, !"frame-pointer", i32 1}
!5 = !{!"Homebrew clang version 21.1.2"}
!6 = distinct !{!6, !7}
!7 = !{!"llvm.loop.mustprogress"}
