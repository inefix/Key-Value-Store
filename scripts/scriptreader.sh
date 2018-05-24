echo "creating reader"
cd ..

./client << EOF
p
r 0
r 1
q
EOF
sleep 8
