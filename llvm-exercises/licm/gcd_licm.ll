; ModuleID = '../gcd_canonical.ll'
source_filename = "gcd.c"
target datalayout = "e-m:o-i64:64-i128:128-n32:64-S128"
target triple = "arm64-apple-macosx14.0.0"

; Function Attrs: noinline nounwind ssp uwtable(sync)
define i32 @gcd(i32 noundef %0, i32 noundef %1) #0 {
  %3 = icmp ne i32 %1, 0
  br i1 %3, label %.lr.ph, label %7

.lr.ph:                                           ; preds = %2
  br label %4

4:                                                ; preds = %4, %.lr.ph
  %.03 = phi i32 [ %0, %.lr.ph ], [ %.012, %4 ]
  %.012 = phi i32 [ %1, %.lr.ph ], [ %5, %4 ]
  %5 = srem i32 %.03, %.012
  %6 = icmp ne i32 %5, 0
  br i1 %6, label %4, label %._crit_edge, !llvm.loop !5

._crit_edge:                                      ; preds = %4
  %split = phi i32 [ %.012, %4 ]
  br label %7

7:                                                ; preds = %._crit_edge, %2
  %.0.lcssa = phi i32 [ %split, %._crit_edge ], [ %0, %2 ]
  ret i32 %.0.lcssa
}

attributes #0 = { noinline nounwind ssp uwtable(sync) "frame-pointer"="non-leaf" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="apple-m1" "target-features"="+aes,+crc,+dotprod,+fp-armv8,+fp16fml,+fullfp16,+lse,+neon,+ras,+rcpc,+rdm,+sha2,+sha3,+v8.1a,+v8.2a,+v8.3a,+v8.4a,+v8.5a,+v8a,+zcm,+zcz" }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"uwtable", i32 1}
!3 = !{i32 7, !"frame-pointer", i32 1}
!4 = !{!"Homebrew clang version 17.0.6"}
!5 = distinct !{!5, !6}
!6 = !{!"llvm.loop.mustprogress"}
