#!/bin/sh

Setup=yes
Cleanup=no
configname=""
while test "x$1" != "x"; do
   case $1 in 
      "-v" ) verbose=x; shift ;;
      "-h" ) help=x; shift;;
      "-cintdlls") cintdlls=x; shift;;
      "-mail") mail=x; shift; mailto=$1; shift;;
      "--config") shift; configname=-$1; shift;;
      --cleanup) Cleanup=yes shift;;
      *) help=x; shift;;
   esac
done

if test "x$help" != "x"; then
    echo "$0 [options]"
    echo "Option:"
    echo "  -v : verbose"
    echo "  -cintdlls : also built the cintdlls"
    echo "  -config configname"
    echo "  --cleanup : if set, this will terminate any unfinished run "
    exit
fi

if [ "x$verbose" = "xx" ] ; then
   set -x
fi

host=`hostname -s`
dir=`dirname $0`

config_filename=$dir/run_roottest.$host$configname.config
sid_filename=run_roottest.$host$configname.sid

if [ $Cleanup = "yes" ] ; then
  echo "Checking for previous run."
  current_sid=`ps h -o sid --pid $$`

  if [ -e $sid_filename ] ; then
     prev_sid=`cat $sid_filename `
     inclusions=" -e root.exe -e run_roottest -e make -e cint "
     exclusions=" -e $$ -e grep "
     # Need to go through a file so that we don't see the temporary sub-shell in the list of files
     ps -s $prev_sid h -o pid,command > /var/tmp/run_roottest.tmp.$$
     old_pids=`cat /var/tmp/run_roottest.tmp.$$ | grep -v $exclusions | grep $inclusions | cut -d' ' -f1 | tr '\n' ' ' `
     rm /var/tmp/run_roottest.tmp.$$
     if [ x"$old_pids" != x ] ; then 
        echo "The previous run_roottest for id $id is still running.  We will terminate it to start a new one."
#        ps -s $prev_sid h -o pid,command
	 kill -9 $old_pids
     fi
  fi
  echo $current_sid > $sid_filename
#  echo Current pids:
#  ps -s $current_sid h -o pid,command
fi

# No sub-process should ever used up more than one hour of CPU time.
ulimit -t 3600

MAKE=gmake
#MAKE=echo
ROOT_MAKEFLAGS=
ROOTTEST_MAKEFLAGS=
CONFIGURE_OPTION="--enable-roofit --enable-tmva "

ROOTMARKS=n/a
FITROOTMARKS=n/a

SHOW_TOP=yes
UPLOAD_LOCATION=flxi06.fnal.gov:/afs/.fnal.gov/files/expwww/root/html/roottest/
UPLOAD_SYNC="ssh -x flxi06.fnal.gov bin/flush_webarea"
SVN_HOST=http://root.cern.ch
SVN_BRANCH=trunk
unset ROOTSYS

# The config is expected to set ROOTLOC,
# ROOTTESTLOC and any of the customization
# above (MAKE, etc.)
# and the method to acquire the `load`
. $config_filename

if [ -z $ROOTSYS ] ; then 
  export ROOTSYS=${ROOTLOC}
  export PATH=${ROOTSYS}/bin:${PATH}
  if [ -z ${LD_LIBRARY_PATH} ] ; then 
    export LD_LIBRARY_PATH=${ROOTSYS}/lib:.
  else 
    export LD_LIBRARY_PATH=${ROOTSYS}/lib:${LD_LIBRARY_PATH}:.
  fi
  export PYTHONPATH=${ROOTSYS}/lib
fi

mkdir -p $ROOTLOC
cd $ROOTLOC

export ROOTBUILD=opt

echo "Running the nightly test on $host$configname from $ROOTLOC"
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:.
export PATH

#echo The path is $PATH
#echo The library path is $LD_LIBRARY_PATH

upload_sync() {
    if [ "x$UPLOAD_SYNC" != "x" ] ; then
       eval $UPLOAD_SYNC
    fi
}

error_handling() {
    cd $ROOTSYS
    write_summary
    upload_log summary.log

    echo "Found an error on \"$host$configname\" ("`uname`") in $ROOTLOC"
    echo "Error: $2"
    echo "See full log file at http://www-root.fnal.gov/roottest/summary.shtml"

    if [ "x$mail" = "xx" ] ; then
	mail -s "root $OSNAME test on `date +"%F"` " $mailto <<EOF
Failure while building root and roottest on $host$configname ("`uname`") in $ROOTLOC
Error: $2
See full log file at http://www-root.fnal.gov/roottest/summary.shtml
EOF
    fi
    upload_sync
    exit $1
}

upload_log() {    
    target_name=$2$1.$host$configname
    scp $1 $UPLOAD_LOCATION/root-today/$target_name > scp.log 2>&1
    result=$?
    if test $result != 0; then 
        cat scp.log 
    fi
}

upload_datafile() {
    target_name=$host$configname.`basename $1`
    scp $1 $UPLOAD_LOCATION/root-today/$target_name > scp.log 2>&1 
    result=$?
    if test $result != 0; then 
        cat scp.log 
    fi
}

one_line() {
   ref=$1
   status=$2

   sline="<td style=\"width: 100px; background:lime; text-align: center;\" >"
   nline="<td style=\"width: 100px; background:gray; text-align: center;\" >"
   fline="<td style=\"width: 100px; background:orange; text-align: center;\" >"
   rline="</td>"

   if test "x$status" = "x$success"; then
      line="$sline <a href="root-today/$ref">$status</a>           $rline"
   elif test "x$status" = "x$na"; then
      line="$nline <a href="root-today/$ref">$status</a>           $rline"
   else
      line="$fline <a href="root-today/$ref">$status</a>           $rline"
   fi
   echo $line
}

write_summary() {
   lline="<td style=\"width: 100px; text-align: center;\" >"
   rline="</td>"
         osline="$lline $OSNAME$configname $rline"
        cvsline=`one_line cvsupdate.log.$host$configname $cvsstatus`
      gmakeline=`one_line gmake.log.$host$configname $mainstatus`
       testline=`one_line test_gmake.log.$host$configname $teststatus`
     stressline=`one_line speedresult.log.$host$configname  $teststatus`
   roottestline=`one_line roottest_gmake.log.$host$configname $rootteststatus`
 roottimingline=`one_line $host$configname.roottesttiming.root $rootteststatus`
 logsbundleline=`one_line $host$configname.logs.tar.gz $rootteststatus`

   date=`date +"%b %d %Y"`
   dateline="$lline $date $rline"
   
   echo $osline         >  $ROOTSYS/summary.log
   echo $cvsline        >> $ROOTSYS/summary.log
   echo $gmakeline      >> $ROOTSYS/summary.log
   echo $testline       >> $ROOTSYS/summary.log
   echo $stressline     >> $ROOTSYS/summary.log
   echo $roottestline   >> $ROOTSYS/summary.log
   echo $roottimingline >> $ROOTSYS/summary.log
   echo $logsbundleline >> $ROOTSYS/summary.log
   echo $dateline       >> $ROOTSYS/summary.log
}

na="N/A"
success="Ok."
failure="Failed"
cvsstatus=$na
mainstatus=$na
teststatus=$na
rootteststatus=$na

mkdir -p $ROOTSYS
cd $ROOTSYS/..
locname=`basename $ROOTSYS`
svn co $SVN_HOST/svn/root/$SVN_BRANCH $locname > $locname/cvsupdate.log  2>&1
result=$?
if test $result != 0; then 
    cvsstatus=$failure
else
    cvsstatus=$success
fi
cd $locname
upload_log cvsupdate.log

cd $ROOTSYS

if test ! -e config.status ; then
    ./configure $CONFIGURE_OPTION > configure.log 2>&1
else
    ./configure `cat config.status` > configure.log 2>&1
fi

$MAKE $ROOT_MAKEFLAGS  > gmake.log  2>&1 
result=$?
upload_log gmake.log
if test $result != 0; then 
   mainstatus=$failure
   error_handling $result "ROOT's gmake failed!  See log file at $ROOTSYS/gmake.log"
fi
mainstatus=$success

if [ "x$cintdlls" = "xx" ] ; then
   gmake cintdlls  >> gmake.log  2>&1 
fi

$MAKE map  >> gmake.log  2>&1 

upload_log gmake.log

cd test; $MAKE distclean > gmake.log
$MAKE >> gmake.log  2>&1 
result=$?

upload_log gmake.log test_
if test $result != 0; then
   teststatus=$failure
   error_handling $result "ROOT's test gmake failed!  See log file at $ROOTSYS/gmake.log"
fi
teststatus=$success

echo >> speedresult.log
date >> speedresult.log
echo Expected rootmarks: $ROOTMARKS | tee -a speedresult.log
./stress -b 30 | tail -4 | grep -v '\*\*\*' | tee -a speedresult.log
./stress -b 30 | tail -4 | grep -v '\*\*\*' | tee -a speedresult.log
echo
if test "x$SHOW_TOP" = "xyes"; then
   top n 1 b | head -14 | tail -11 | tee -a speedresult.log
fi
if test "x$$IDLE_COMMAND" != "x"; then
   idle=`eval $IDLE_COMMAND`
   echo "idle value: $idle" | tee -a speedresult.log
fi
echo
echo Expected fit rootmarks: $FITROOTMARKS
./stressFit | tail -5 | grep -v '\*\*\*' | grep -v 'Time at the' | tee -a speedresult.log
./stressFit | tail -5 | grep -v '\*\*\*' | grep -v 'Time at the' | tee -a speedresult.log
echo

upload_log speedresult.log

echo Going to roottest at: $ROOTTESTLOC

mkdir -p $ROOTTESTLOC
cd $ROOTTESTLOC/..
locname=`basename $ROOTTESTLOC`
svn co $SVN_HOST/svn/roottest/$SVN_BRANCH $locname > $locname/gmake.log 2>&1

cd $ROOTTESTLOC
$MAKE clean >> gmake.log 2>&1 
$MAKE -k >> gmake.log 2>&1 
result=$?
upload_log gmake.log roottest_

gmake logs.tar.gz >> gmake.log 2>&1

upload_datafile roottesttiming.root
upload_datafile logs.tar.gz

grep FAIL $PWD/gmake.log
tail $PWD/gmake.log

if test $result != 0; then
    rootteststatus=$failure
    error_handling $result "roottest's gmake failed!  See log file at $PWD/gmake.log"
fi
rootteststatus=$success


cd $ROOTSYS
write_summary
upload_log summary.log

upload_sync
