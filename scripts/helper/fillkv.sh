echo "creating writer"
cd ../..
./client << EOF
a test
a test2
a test3
a test4
a test5
p
s
p
q
EOF
