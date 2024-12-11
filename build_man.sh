#!/bin/sh
[ -n "$BASH" ] || exec bash -p $0 $@

# Script to process man pages output by doxygen.
# We need to use bash for its associative array facility.
# (`bash -p` prevents import of functions from the environment).
# Args: none or 2 being man7 page name & relative path of source with \mainpage
# (i.e. relative to first directory in INPUT line of Doxyfile)

declare -A renamed_page
done_synopsis=false
have_macros=false
have_functions=false
real_pages=
man_links=

# Array of ed commands used to compact the synopsis.
# Name chosen to be unique (to grep -w)
for ((i=8;i>=0;i--)); do y[$i]=#; done
y[9]=wq

main(){
  set -e
  # make_man7 has no dependencies or dependants so kick it off now
  [ $# -ne 2 ] || make_man7 $@ &
  pushd man/man3 >/dev/null; rm -f _*
  count_real_pages
  rename_real_pages
  # Nothing depends on make_symlinks so background it
  make_symlinks $man_links &
  post_process $@
  wait
}

count_real_pages(){
  page_count=0
  #
  # Count "real" man pages (i.e. not generated by MAN_LINKS)
  # MAN_LINKS pages are 1-liners starting .so
  # Method: list files in descending order of size,
  # looking for the first 1-liner
  #
  for i in $(ls -S)
  do head -n1 $i | grep -E -q '^\.so' && break
    page_count=$(($page_count + 1))
  done
}

rename_real_pages(){
  for i in $(ls -S | head -n$page_count)
  do
    if grep -Eq 'Functions|Macros' $i; then
      j=$(ed -s $i <<////
/Functions\|Macros/+1;.#
/^\.RI/;.#
.,.s/^.*\\\\fB//
.,.s/\\\\.*//
.,.w /dev/stdout
Q
////
).3
      mv -f $i $j
      real_pages="$real_pages $j"
    else
      # Page has neither functions nor macros.
      # Do not process further, but symlink to it if appropriate.
      j=$i
    fi
    renamed_page[$i]=$j
  done
  man_links=$(ls -S | tail -n+$(($page_count + 1)))
}

make_symlinks(){
  for j in $@
  do ln -sf ${renamed_page[$(cat $j | cut -f2 -d/)]} $j
  done
}

post_process(){
  #
  # DIAGNOSTIC / DEVELOPMENT CODE
  # set -x and restrict processing to keep_me: un-comment to activate
  # Change keep_me as required
  #
  #keep_me=nfq_icmp_get_hdr.3
  #do_diagnostics
  #
  # Work through the "real" man pages
  for target in $real_pages
  do grep -Eq '^\.SH "(Function|Macro Definition) Documentation' $target ||
    continue

    {
      grep -Eq '^\.SS "Macros' $target && have_macros=true
      grep -Eq '^\.SS "Functions' $target && have_functions=true
      remove_first_synopsis
      fix_name_line
      $done_synopsis && move_synopsis
      del_empty_det_desc
      del_def_at_lines
      fix_double_blanks
      [ $# -ne 2 ] || insert_see_also $@
    }&

  done

}

make_man7(){

  # This grep command works for multiple directories on the INPUT line,
  # as long as the directory containing the source with the main page
  # comes first.
  target=/$(grep -Ew INPUT Doxyfile | cut -f2- -d/ | cut -f1 -d' ')/$2
  mypath=$(dirname $0)

  # Build up temporary source in temp.c
  # (doxygen only makes man pages from .c files).
  ed -s $target << ////
1,/\\\\mainpage/d
0i
/**
 * \\defgroup $1 $1 overview
.
/\\*\\//+1,\$d
a

/**
 * @{
 *
 * $1 - DELETE_ME
 */
int $1(void)
{
	return 0;
}
/**
 * @}
 */
.
wq temp.c
////

  # Create temporary doxygen config in doxytmp
  grep -Ew PROJECT_NUMBER Doxyfile >doxytmp
  cat >>doxytmp <<////
PROJECT_NAME = $1
ABBREVIATE_BRIEF =
FULL_PATH_NAMES = NO
TAB_SIZE = 8
OPTIMIZE_OUTPUT_FOR_C = YES
EXAMPLE_PATTERNS =
ALPHABETICAL_INDEX = NO
SEARCHENGINE = NO
GENERATE_LATEX = NO
INPUT = temp.c
GENERATE_HTML = NO
GENERATE_MAN = YES
MAN_EXTENSION = .7
////

  doxygen doxytmp >/dev/null

  # Remove SYNOPSIS line if there is one
  target=man/man7/$1.7
  mygrep "SH SYNOPSIS" $target
  [ $linnum -eq 0 ] || delete_lines $linnum $((linnum+1))

  # doxygen 1.8.9.1 and possibly newer run the first para into NAME
  # (i.e. in this unusual group). There won't be a SYNOPSIS when this happens
  if grep -Eq "overview$1" $target; then
    echo "Re-running doxygen $(doxygen --version)"
    ed -s temp.c << ////
2a
 * \\manonly
.PP
.SH "Detailed Description"
.PP
\\endmanonly
.
wq
////
    doxygen doxytmp >/dev/null
  fi

  rm temp.c doxytmp
}

# Insert top-level "See also" of man7 page in man3 page
insert_see_also(){
  mygrep "Detailed Description" $target
  [ $linnum -ne 0 ] || mygrep "Function Documentation" $target
  [ $linnum -ne 0 ] || { echo "NO HEADER IN $target" >&2; return; }
  ed -s $target <<////
${linnum}i
.SH "See also"
\\fB${1}\\fP(7)
.
wq
////
}

fix_double_blanks(){
  linnum=1
  #
  # Older versions of man display a blank line on encountering "\fB\fP";
  # newer versions of man do not.
  # doxygen emits "\fB\fP" on seeing "\par" on a line by itself.
  # "\par" gives us double-spacing in the web doc, which we want, but double-
  # spacing looks odd in a man page so remove "\fB\fP".
  #
  while [ $linnum -ne 0 ]
  do mygrep \\\\fB\\\\fP $target
    [ $linnum -eq 0 ] || delete_lines $linnum $linnum
  done
}

del_def_at_lines(){
  linnum=1
  while [ $linnum -ne 0 ]
  do mygrep '^Definition at line (\\fB)?[[:digit:]]*(\\fP)? of file' $target
    [ $linnum -eq 0 ] || delete_lines $(($linnum - 1)) $linnum
  done
}

# Only invoked if you un-comment the 2 diagnostic / development lines above
do_diagnostics(){
  mv $keep_me xxx
  rm *.3
  mv xxx $keep_me
  page_count=1
  real_pages=$keep_me
  set -x
}

del_empty_det_desc(){
  mygrep "^\\.SH \"Function Documentation" $target
  i=$linnum
  mygrep "^\\.SH \"Detailed Description" $target
  [ $linnum -ne 0  ] || return 0
  [ $(($i - $linnum)) -eq 3 ] || return 0
  # A 1-line Detailed Description is also 3 lines long,
  # but the 3rd line is not empty
  i=$(($i -1))
  [ $(tail -n+$i $target | head -n1 | wc -c) -le 2 ] || return 0
  delete_lines $linnum $i
}

move_synopsis(){
  if $have_macros
  then
    y[0]='/^\.SS "Macros"/;.d'
    y[1]=.ka
    y[2]='/^\.SH SYNOPSIS/;/^[[:space:]]*$/-1m'\''a-1'
    y[3]='/"Macro Definition Documentation"/-1;.d'

    # need to delete 4 lines if we have functions as well
    $have_functions && y[4]='/^\.SS "Functions"/;.-1,.+2d'
  else
    y[5]='/^\.SS "Functions"/;.d'
    y[6]=.ka
    y[7]='/^\.SH SYNOPSIS/;/^[[:space:]]*$/-1m'\''a-1'
    y[8]='/"Function Documentation"/-1;.d'
  fi
  ed -sv $target <<////
${y[0]}
${y[1]}
${y[2]}
${y[3]}
${y[4]}
${y[5]}
${y[6]}
${y[7]}
${y[8]}
${y[9]}
////
}

fix_name_line(){
  all_funcs=""

  # Search a shortened version of the page in case there are .RI lines later
  mygrep '^\.SH "(Function|Macro Definition) Documentation"' $target
  head -n$linnum $target >../$target.tmp

  while :
  do foundline=$(grep -En '^\.RI' ../$target.tmp 2>/dev/null | head -n1)
    [ "$foundline" ] || break
    linnum=$(echo $foundline | cut -f1 -d:)
    # Discard this entry (and all previous lines)
    ed -s ../$target.tmp <<////
1,${linnum}d
wq
////

    func=$(echo $foundline | cut -f2 -d\\ | cut -c3-)
    [ -z "$all_funcs" ] && all_funcs=$func ||\
      all_funcs="$all_funcs, $func"
  done
  # For now, assume name is at line 5
  desc=$(head -n5 $target | tail -n1 | cut -f3- -d" ")
  ed -s $target <<////
5c
$all_funcs \\- $desc
.
wq
////
  rm ../$target.tmp
}

remove_first_synopsis(){
  # If the user has inserted a manonly synopsis with headers then we don't
  # need the synopsis that doxygen created.
  # Removing that synopsis has the side benefit that we get rid of any Topics
  # that may be listed.
  #
  [ $(grep -E 'SH SYNOPSIS' $target | wc -l) -eq 2 ] || return 0
  ed -s $target <<////
/SH SYNOPSIS/,/^\\.SS \"\(Functions\|Macros\)/-1d
wq
////
  done_synopsis=true
}

# Delete lines $1 through $2 from $target
delete_lines(){
  ed -s $target <<////
$1,$2d
wq
////
}

mygrep(){
  linnum=$(grep -En "$1" $2 2>/dev/null | head -n1 | cut -f1 -d:)
  [ $linnum ] || linnum=0
}

main $@
