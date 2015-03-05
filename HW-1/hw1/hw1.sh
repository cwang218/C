choice=0

while [ $choice -ne 4 ]
do
	echo "---------------------------------------------------------------------------"
	echo " Menu "
	echo " 1. Print the ancestry tree of the shell script currently running."
	echo " 2. Print online usernames."
	echo " 3. Print the processes any user is running."
	echo " 4. Quit"
	echo " "
	
	read choice
	echo " "
	case $choice in
	1)
		ps -ef > processes.txt
		currID=$(cat processes.txt | grep 'sh hw1.sh' | cut -c10-15 | head -1)
		echo $currID > output.txt
		parID=$(cat processes.txt | grep 'sh hw1.sh' | cut -c16-22 | head -1)
		cat processes.txt | cut -c10-20 > processes2.txt
		while [ $currID -ne 1 ]
		do
			echo '  |' >> output.txt		
			parID=$(cat processes2.txt | grep $currID | cut -c7-12 | sort -n | head -1)
			currID=$parID
			echo $currID >> output.txt
		done
		cat output.txt
		;;		
	2)
		who | cut -c1-6 | sort -u
		;;  
	3)
		who | cut -c1-6 | sort -u > userlist.txt
		echo " Select a user: "
		cat userlist.txt -n
		echo " "
	
		read usernum
		cuser=$(cat userlist.txt | head -$usernum | tail -1)
		echo " "
		ps -fu $cuser
		;;
	esac

	echo " "
	echo " "
done
