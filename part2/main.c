#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

struct inode {
    char* name; //8 bytes
    int size;
    int* blockPointer; //Size 8
    int used;
};

struct inode* new_inode() {
    struct inode* retVal = malloc(sizeof(struct inode));
    if(retVal == NULL) return NULL;

    retVal->name = malloc(8*sizeof(char));
    retVal->blockPointer = malloc(8*sizeof(int));
    if(retVal->name == NULL || retVal->blockPointer == NULL) {
        free(retVal);
        return NULL;
    }

    return retVal;
}

void del_inode(struct inode* i) {
    if(i != NULL) {
        free(i->blockPointer);
        free(i);
    }
}

void write_inode(struct inode* node, int node_num, FILE* f) {
    int start = ((node_num * 8) - 8) * 1024;
    //Move pointer to the index
    fseek(f, start, SEEK_SET);
    //Write the inode data
    char *dummy = malloc(sizeof(char));

    for (int i = 0; i < 8; i++) {
        //Set dummy based on block pointer value
        if (node->blockPointer[i] == 1) {
            dummy[0] = 1;
        } else {
            dummy[0] = 0;
        }

        //Write dummy 1KB times (either 0 or 1 based on block pointer value)
        for (int j = 0; j < 1024; j++) {
            fwrite(dummy, 1, 1, f);
        }
    }

    fflush(f);
}

void write_superblock(struct inode** nodes, FILE* f) {
    //Go to start of file
    fseek(f, 0, SEEK_SET);
    //Write block use values
    char* dummy = malloc(sizeof(char));
    for(int i = 0; i < 16; i++) {
        for(int j = 0; j < 8; j++) {
            if(nodes[i]->blockPointer[j] == 1) {
                dummy[0] = 1;
            } else {
                dummy[0] = 0;
            }

            fwrite(dummy, 1, 1, f);
        }
    }

    //Write inode data
    for(int i = 0; i < 16; i++) {
        //Write name
        fwrite(nodes[i]->name, sizeof(char), 8, f);
        //Write size
        fwrite((const void *) &(nodes[i]->size), sizeof(int), 1, f);
        //Write block pointers
        fwrite((const void *)nodes[i]->blockPointer, sizeof(int), 8, f);
        //Write used
        fwrite((const void *)&(nodes[i]->used), sizeof(int), 1, f);
    }

    fflush(f);
}


struct inode** load(FILE* f) {
    struct inode* temp = new_inode();
    struct inode** nodes = malloc(16*sizeof(*temp));
    free(temp);
    if(nodes == NULL) {
        printf("Failed to allocate memory for inodes!");
        return NULL;
    }
    if(f==NULL) {
        printf("Failed to open disk %d!\n", errno);
        return NULL;
    }
    char* super_block = (char*)malloc(128*sizeof(char));
    fread(super_block, 128, 1, f);

    int idx = 0;
    //16 inodes
    for(int i = 0; i < 16; i++) {
        nodes[i] = new_inode();
        //8 blocks per inode
        for(int j = 0; j < 8; j++) {
            nodes[i]->blockPointer[j] = super_block[idx];
            //If any of the blocks are used, this inode is already used
            if(super_block[idx] == 1) {
                nodes[i]->used = 1;
            }
            idx++;
        }
    }

    //Read more inode data
    for(int i = 0; i < 16; i++) {
        struct inode* node = nodes[i];
        //Read name (8 bytes)
        fread(node->name, sizeof(char), 8, f);
        //Read file size
        fread(&node->size, sizeof(int), 1, f);
        //Skip block pointers
        fseek(f, 8*sizeof(int), SEEK_CUR);
        //Skip used
        fseek(f, sizeof(int), SEEK_CUR);
    }

    free(super_block);

    return nodes;
}

int create(char name[8], int size, struct inode** nodes, FILE* disk) {
    //Max file size 8 blocks
    if(size > 8) {
        printf("Size cannot exceed 8 blocks for create!\n");
        return -1;
    }
    //Look for a free node
    struct inode* free_node = NULL;
    int node_num = -1;
    for(int i = 0; i < 16; i++) {
        if(nodes[i]->used == 0) {
            free_node = nodes[i];
            node_num = i+1;
            break;
        }
    }

    //Failed to find a free node
    if(free_node == NULL) {
        printf("Failed to find free node for create\n");
        return -1;
    }
    //Look for duplicate name
    for(int i = 0; i < 16; i++) {
        if(strcmp(nodes[i]->name, name) == 0) {
            printf("Cannot create duplicate file %s and %s!\n", nodes[i]->name, name);
            return -1;
        }
    }

    //Mark node as used and update node blocks
    free_node->used = 1;
    free_node->size = size;
    free_node->name = name;
    for(int i = 0; i < size; i++) {
        free_node->blockPointer[i] = 1;
    }

    //Write inode
    write_inode(free_node, node_num, disk);

    //Write super block
    write_superblock(nodes, disk);

    return 0;
}

int delete(char name[8], struct inode** nodes, FILE* disk) {
    struct inode* node = NULL;
    int node_num = -1;

    for(int i = 0; i < 16; i++) {
        if(strcmp(nodes[i]->name, name) == 0) {
            node = nodes[i];
            node_num = i+1;
        }
    }

    //Node not found, can't delete it
    if(node == NULL) {
        printf("Cannot delete non-existent file!\n");
        return -1;
    }

    //Set node used to 0
    node->used = 0;
    //Set node blocks to 0
    for(int i = 0; i < 8; i++) node->blockPointer[i] = 0;
    //Set node size to 0
    node->size = 0;
    //Set node name to null chars
    for(int i = 0; i < 8; i++) node->name[i] = '\0';

    //Update node in disk
    write_inode(node, node_num, disk);

    //Update super block
    write_superblock(nodes, disk);

    return 0;
}

int ls(struct inode** nodes) {
    //i=1 to skip super block
    for(int i = 1; i < 16; i++) {
        if(nodes[i]->used == 1) {
            printf("Name: %s Size: %d\n", nodes[i]->name, nodes[i]->size);
        }
    }

    return 0;
}

int read(char name[8], int block_num, char buf[1024], struct inode** nodes, FILE* disk) {
    if(block_num < 0 || block_num > 7) return -1;

    struct inode* node = NULL;
    int node_num = -1;

    for(int i = 0; i < 16; i++) {
        if(strcmp(nodes[i]->name, name) == 0) {
            node = nodes[i];
            node_num = i+1;
        }
    }

    //Node not found
    if(node == NULL) {
        printf("Cannot read non-existent file!\n");
        return -1;
    }

    //Find start index of the block in file
    int start = ((node_num * 8) - 8) * 1024 + block_num*1024;
    fseek(disk, start, SEEK_SET);

    //Read into buffer
    fread(buf, sizeof(char), 1024, disk);

    return 0;
}

int write(char name[8], int block_num, char buf[1024], struct inode** nodes, FILE* disk) {
    if(block_num < 0 || block_num > 7) {
        printf("Block num out of range\n");
        return -1;
    }

    struct inode* node = NULL;
    int node_num = -1;

    for(int i = 0; i < 16; i++) {
        if(strcmp(nodes[i]->name, name) == 0) {
            node = nodes[i];
            node_num = i+1;
        }
    }

    //Node not found
    if(node == NULL) {
        printf("File not found for write\n");
        return -1;
    }

    //Find start index of the block in file
    int start = ((node_num * 8) - 8) * 1024 + block_num*1024;
    fseek(disk, start, SEEK_SET);

    //Read into buffer
    fwrite(buf, sizeof(char), 1024, disk);

    return 0;
}

int main() {

    FILE* f = fopen("lab3input.txt", "r");
    if(f == NULL) {
        printf("Error opening lab3input.txt!\n");
        return -1;
    }

    char line[60];
    char* name = NULL;
    if(fgets(line, 60, f) != NULL) {
        unsigned long len = strlen(line);
        if(line[len-1] == '\n')
            line[len-1] = '\0'; /* Remove new line */
        if(line[len-2] == '\r')
            line[len-2] = '\0'; /* Remove carriage return */
        name = malloc(len);
        for(int i = 0; i < len; i++) name[i] = line[i];
    }

    if(name == NULL) {
        printf("Unable to read filename from input.");
        return -1;
    }

    FILE* disk = fopen(name, "rb+");
    if(!disk) {
        printf("Can't open disk!");
        return -1;
    }

    struct inode** nodes = load(disk);

    char* cCommand = "C"; char* dCommand = "D"; char* rCommand = "R"; char* wCommand = "W"; char* lCommand = "L";
    while(fgets(line, 60, f) != NULL) {
        unsigned long len = strlen(line);
        if(line[len-1] == '\n')
            line[len-1] = '\0'; /* Remove new line */
        if(line[len-2] == '\r')
            line[len-2] = '\0'; /* Remove carriage return */
        char* token = strtok(line, " ");
        int idx = 0;
        char* components[3];
        while( token != NULL ) {
            //printf("%s\n", token);
            components[idx++] = token;
            token = strtok(NULL, " ");
        }


        if (strcmp(components[0], cCommand) == 0) {
           create(components[1], atoi(components[2]), nodes, disk);
        } else if (strcmp(components[0], dCommand) == 0) {
            delete(components[1], nodes, disk);
        } else if (strcmp(components[0], rCommand) == 0) {
            char buf[1024];
            read(components[1], atoi(components[2]), buf, nodes, disk);
            printf("Read the following from the block:\n");
            for(int i = 0; i < 1024; i++) printf("%d", buf[i]);
            printf("\n");
        } else if (strcmp(components[0], wCommand) == 0) {
            char buf[1024];
            write(components[1], atoi(components[2]), buf, nodes, disk);
        } else if (strcmp(components[0], lCommand) == 0) {
            ls(nodes);
        }

    }

    fclose(disk);
    fclose(f);
    for(int i  = 0; i < 16; i++) {
        del_inode(nodes[i]);
    }
    free(nodes);
}
