
#TASK 1A

We need to ping some host.

Let's take google.com as our host

Since DNS is not used in the above code or anywhere we need to pass ip address as the argument while executing the file

google.com ip address is 8.8.8.8. let's ping this ip address

Type the following commands to compile and execute ping1a program

1.g++ ping1a.cpp -o ping1a

2.sudo ./ping1a 8.8.8.8

We need to use sudo to invoke root privileges.Else we get errors.You can check yourself


#TASK 1B

The program is almost same as ping1a

The only difference is add message "hello" in icmp echo request packet and print the message in the received packet

Changes happen only in the send_v4 and process_v4 functions.All others remain as it is in ping1a program.

Follow the same instructions inorder to compile and execute the program


#TASK 2

In this program we need to print MAC Address of the local machine.

The default port is eth0 interface for network communication.So added this port to the program.

MAC Address is HWaddr part corresponding to eth0 when u run this command => ifconfig -a in the terminal

Compile and execute the program using these instructions:

1.g++ mac_addr.cpp -o mac_addr

2. ./mac_addr or sudo mac_addr

This should give the mac address as the output in the terminal

NOTE: You can also see the images attached in the folder as to check the outputs of every task executed. :)



