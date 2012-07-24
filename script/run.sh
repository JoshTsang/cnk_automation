#bin/bash
#if [ $(git --version | grep -c "version") -le 0 ]
#then
#    echo "Instal git and ensure Internet is accessable"
#fi

#check modification

#get database
rm ./menu.db
echo download database from server, please:
echo 1.ensure network connection
echo 2.wait a moument cuz password is required
scp root@192.168.1.1:~/db/menu.db ./menu.db
if [ -f menu.db ]
then
 echo "downlaod db succ"
else
 echo "download db failed"
 exit 0
fi
#compile
echo compling...
rm ../bin/test
gcc ../src/test.c -o ../bin/test -lsqlite3

#run cases
echo start running testcases
printf "%10s%15s%10s%10s%10s\r\n" "Case" "Action" "Return" "Result" "Duration"
caseMajors=$(ls ../cases)
for caseMajor in $caseMajors
do
 minors=$(ls ../cases/$caseMajor)
 for caseMinor in $minors
 do
   ../bin/test $caseMajor.$caseMinor $(cat ../cases/$caseMajor/$caseMinor)
 done
done
