all:
	g++ -o awsoutput aws.cpp -std=c++17
	g++ -o serverAoutput serverA.cpp -std=c++17
	g++ -o serverBoutput serverB.cpp -std=c++17
	g++ -o client client.cpp -std=c++17

aws:
	./awsoutput

serverA:
	./serverAoutput

serverB:
	./serverBoutput
