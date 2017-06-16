{
  if (length > 0) {
    txt=$2""
    printf "%d	%d  %s\n", $1, gsub("/", "/", $2), $2
  }
  else
    printf "\n"
}
