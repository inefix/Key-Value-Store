echo "creating writer"
cd ..
./client << EOF
a test1
a test2
a test3
a test4
a test5
m 1 test7
d 2
p
q

EOF

sleep 8
