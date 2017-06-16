# sums up the numbers in field N and prints it at the end of input
#
# You must use "awk -f $cmds/sumN.awk -v N=<fld>" where <fld> is the number of the
# field you want to total, and 1 <= <fld> <= 10
#
BEGIN { total = 0 }

{
  if (length > 0) {
    total = total + $N
  }
  printf "%s\n", $0
}

END { printf "%d total for field %d\n", total, N }
