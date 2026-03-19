# Remote-pc-controller

**Remark** This project is ment for educaiton and a project combining between C knowledge , Operating systems , and Network programming. Not production level , i'd personally say it does its job.
[Preview](https://cdn.discordapp.com/attachments/1248401913586651141/1484293393545953320/fee1c550-95a9-4bdf-ab06-7c24d7790120.mov?ex=69bdb371&is=69bc61f1&hm=02899fc94d148ac777749b6bef76225d7eadb50f80911ca2f240cdb5fb62905c&)\
Another thing worth mentioning is that for those who want the program to be windowless or hidden , you can use the `WinMain` as a main function and pair it with `ShowWindow` function to hide the window

### How to setup
1. Download the source code or pre built binary
2. Download setvol
3. Add setvol to User's enviromental paths
4. Feel free to add features in the server's "handleRequest" function
5. Feel free to change the server's UI by modifying the `index.html` `style.css` `script.js`
---
### How to use
1. Turn on the server
2. Access the website throught localhost or current IPv4 IP of the machine the server is on
3. Make sure to disable Firewall to allow connection
4. Remember its port 80 , HTTP protocol
---
### Features
The server is a normal server operating locally accepting http connections , it handles clients through a private thread pool (Max threads: 50) which you can be monitored using an Environment , but that wasnt our case. Featuring a small request parser, not an HTTP handler. 

I'd say the most important feature is leveraging Thread pools instead of normal threads for clients with the use of ASYNC i/o for the coming reasons:

- Thread pools minimize the cost of thread creation and destruction since both are heavy operations
- They help manage the workload by allowing tasks to be queued up until a thread becomes available, preventing system overload
- Once a task is completed, the thread can be returned to the pool and used for another task, promoting efficient thread reuse


#### Links
[SetVol Tool](https://github.com/roblatour/setvol)\
[Preview](https://cdn.discordapp.com/attachments/1248401913586651141/1484293393545953320/fee1c550-95a9-4bdf-ab06-7c24d7790120.mov?ex=69bdb371&is=69bc61f1&hm=02899fc94d148ac777749b6bef76225d7eadb50f80911ca2f240cdb5fb62905c&)
