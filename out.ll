; ModuleID = "techflow"
target triple = "x86_64-pc-linux-gnu"
target datalayout = ""

declare i32 @"printf"(i8* %".1", ...)

@"fmt" = internal constant [4 x i8] c"%d\0a\00"
define void @"main"()
{
entry:
  %"x" = alloca i32
  store i32 42, i32* %"x"
  %".3" = load i32, i32* %"x"
  %".4" = bitcast [4 x i8]* @"fmt" to i8*
  %".5" = call i32 (i8*, ...) @"printf"(i8* %".4", i32 %".3")
  ret void
}
