#!/bin/bash
#
# simple starting point to build .ccls from automake Makefile
header()
{
  echo -n "" > ${out}
  local flags=`grep -E "^CXX = " Makefile`
  flags=${flags#*=}
  local p
  local i=0
  for p in $flags
  do
    if [ "$i" -eq 0 ] ; then    # CXX contains compiler as first parameter (or can we stream all of these?)
       echo "${p}" >> ${out}
    fi
    if [[ "$p" == "-std"* ]] ; then
       echo "${p}" >> ${out}
    fi
    ((i+=1))
  done
  #echo "-stdlib=libc++" >> ${out}   # not sure where to find this, is it needed?
}
gccdefault()
{
  #just capture stderr to flags
  local flags=`g++ -E -x c++ - -v < /dev/null 2>&1 > /dev/null`
  #save current ifs state
  unset saved_IFS
  [ -n "${IFS+set}" ] && saved_IFS=$IFS
  #Set the field separator to new line
  IFS=$'\n'
  local in=0
  local p
  for p in $flags
  do
    #echo "${p}"
    if [ "$in" -eq 0 ] ; then
       if [[ "$p" == "#include <...>"* ]] ; then
         in=1
       fi
    else
       if [[ "$p" == "End of"* ]] ; then
         in=0
       else
         echo "-isystem${p}" >> ${out}
       fi
    fi
  done
  #and restore ifs
  unset IFS
  [ -n "${saved_IFS+set}" ] && { IFS=$saved_IFS; unset saved_IFS; }
}
includes()
{
  local flags=`grep -F "_CFLAGS = " Makefile`
  flags=${flags#*=}
  local p
  for p in $flags
  do
    if [[ "$p" == "-I"* ]] ; then
       echo "${p}" >> ${out}
    fi
  done
}
out=.ccls
header
gccdefault
includes
unset out