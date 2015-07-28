Net Centric Computing
Student: Juan C. Riaño
Florida International University



Simple Web Server how to:

To compile run
	make
To execute, provide the application name and a port betwween 1024 ~ 4951
	./ws 1025
point your browser to the server port, and provide a file name if you know it,
or simply leave out the file name to bring up the defalt page.
	http://localhost:1025/

This will bring up the default page. The "website" contents are located in the
"html" folder inside the project folder.

The address in the web browser can be typed in any case, the server will
serve the page ignoring case.

BUGS:
Could not figure out why when serving pdf files to Chrome browser, the
transmission hangs.
In Firefox the pdf documents are displayed just fine.

Sources:
- Class notes
Books:
- Computer Networking  A Top Down Approach 5th Edition
Web:
- Beej's Guide to Network Programming
  http://beej.us/guide/bgnet/output/html/multipage/index.html
