Created by Raghav and Carlos

Compilation Instructions


Description
This program reads a txt file as such:
- Reads each line and splits each string by spaces (the first line reads the file created by the create_fs)
- The remaining are read and stored in a char* arr which give us the command character (which correlates with the file system methods, C for create, D for delete, etc.) name of the file we are modifying in the system and the size or block num of that file
- We have created structures for the inodes, with helper methods that lets us delete and modify them (usefull for the manipulation of the main methods)
- We have created strcutures for the data block, with a helper method that modifies the superblock information (making it easier to modifty the inodes data and block list)
- A create method that looks for a free inode in the superblock section and check to see if no other existing file has the same name then adds the inode data and updates the superblock
- A delete method that finds the inode with the given name file and updates the superblock with default values (indicating there is space for new files)
- A ls (list) method that prints out all the inodes filename and size on each line
- A read method that reads the data of the file (from the in the block allocated by the inode in the superblock) into a buffer
- A write method that writes the data of the file (from the in the block allocated by the inode in the superblock) into a buffer