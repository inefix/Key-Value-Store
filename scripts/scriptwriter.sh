echo "creating writer"
cd ..
./client << EOF
p
a test1
a test2
a test3
m 0 test5
d 1
p
q
EOF

sleep 8
