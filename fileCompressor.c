#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <dirent.h>
#include <limits.h>
#include <fcntl.h>

//WORD OCCURRENCE FUNCTIONS---------------
struct WordOccurrence{
    int freq;
    char* word;
};
typedef struct WordOccurrence WordOcc;

WordOcc* createOcc(int freq, char* word);
void printOcc(WordOcc* occ);
int getFreq_occ(WordOcc* occ);
char* getWord_occ(WordOcc* occ);
//END WORD OCCURRENCE FUNCTIONS------------

//AVL FUNCTIONS----------------------
struct avl_node{
    WordOcc* key;
    struct avl_node* left;
    struct avl_node* right;
    int height;
};
typedef struct avl_node Node;

int get_max(int x, int y);
int get_height(Node* node);
Node* newNode(WordOcc* key);
int avl_get_key_val(Node* node);
char* avl_get_key_word(Node* node);
Node* rotate_right(Node* y);
Node* rotate_left(Node* x);
int get_balance_factor(Node* node);
Node* insert(Node* node, WordOcc* key);
Node* get_smallest_node(Node* node);
Node* avl_remove(Node* root, WordOcc* key);
Node* remove_root(Node* root);
void print_preOrder(Node* root);
int getTreeHeight(Node* root);
int getMaxNumElements(int height);
int count_postOrder(Node* root);


//END AVL FUNCTIONS-------------------


//MIN-HEAP FUNCTIONS----------------------
struct MinHeapNode{
    WordOcc* data;
    struct MinHeapNode *left, *right;
};
typedef struct MinHeapNode MinHeapNode;

struct MinHeap{
    MinHeapNode** arr;
    int size;
    int capacity;
};
typedef struct MinHeap MinHeap;

MinHeapNode* newHeapNode(WordOcc* data);
MinHeap* createMinHeap(int capacity);
void switchHeapNodes(MinHeapNode** x, MinHeapNode** y);
void heapify(MinHeap* heap, int index);
int isOne(MinHeap* heap);
void heap_insert(MinHeap* heap, MinHeapNode* node);
void build_heap(MinHeap* heap);
MinHeapNode* heap_get_min(MinHeap* heap);
int isLeaf(MinHeapNode* root);
MinHeap* makeMinHeap(Node* wordTree, int size);
MinHeapNode* makeHuffmanTree(Node* wordTree, int size);
void printArray(int arr[], int n);
void printCodes(MinHeapNode* root, int arr[], int top);
void huffmanCodes(Node* wordTree, int size);
int getTreeHeight_Huff(MinHeapNode* root);

//END MIN-HEAP FUNCTIONS----------------------


int printFiles(char* directory);

int printDirs(char* bpath);

int isDirectory(char* path);

int isReg(char* path);

int fileType(char* path);

void transferDataToFile(char* read_path, char* write_path);

int find_and_transfer(char* path);

//Transfers contents of all files within parent and sub directories
//Creates write_temp.txt inside of working parent directory
//write_temp.txt will be deleted after the huffman codebook is created
//bpath is the directory you wish to scan, masterPath should be the same as bpath but it remains the same throughout the rescursion process
int rec_transfer(char* bpath, char* masterPath);

void printTree(Node* root, int space);

char* intArray_to_string(int arr[], int size);

//Transfer file contents to write_temp file
//Tokenize and load tokens into avl tree
//build Huffman codes
//Outputs the HuffmanCodebook file
void buildCodeBook(char* flags, char* filename);



int main(int argc, char* argv[]){
    //INPUT FORMAT: ./fileCompressor <flag> <path or file> |codebook|
    if(argc > 5){
        printf("Too many arguments...\n");
        return 0;
    }else if(argc < 3){
        printf("Too few arguments. . .\n");
        return 0;
    }

    

    printf("%s\n", argv[1]);
    if(argv[1][0] != '-'){
        printf("flags must start with '-' . . .\n");
        return 0;
    }else if(strcmp(argv[1], "-R") == 0){
        if(argv[2] == NULL || strcmp(argv[2], "-b") != 0){
            printf("Second argument must be -b...\n");
            return 0;
        }

        if(argv[3] == NULL){
            printf("Third argument must be a directory path. . .\n");
            return 0;
        }else{
            buildCodeBook("-R", argv[3]);
        }
    }else if(strcmp(argv[1], "-b") == 0){
        if(argv[2] == NULL || !isReg(argv[2])){
            printf("Second argument must be a file path...\n");
        }else{
            buildCodeBook("-b", argv[2]);
        }
    }

    if(strcmp(argv[1], "-c") == 0){
        printf("Compression not implemented...\n");
        return 0;
    }
    if(strcmp(argv[1], "-d") == 0){
        printf("Decompression not implemented...\n");
        return 0;
    }

    
}

int printFiles(char* directory){
    printf("FILES IN DIRECTORY:\n");
    DIR *d;
    struct dirent *dir;
    d = opendir(directory);
    if(d){
        while((dir = readdir(d)) != NULL){
            int type = isReg(dir->d_name);
            if(type != 0){
                printf("%s\n", dir->d_name);
            }
        }
        closedir(d);
    }
    printf("--- END FILE LIST ---\n");
    return 0;
}

int printDirs(char* bpath){
    char path[1000];
    struct dirent* dp;
    DIR *d = opendir(bpath);

    if(!d){
        return -1;
    }

    while((dp = readdir(d)) != NULL){
        if(strcmp(dp->d_name,"..") != 0 && strcmp(dp->d_name,".") != 0){
            
            strcpy(path,bpath);
            strcat(path,"/");
            strcat(path, dp->d_name);
            printf("%s\n", path);



            printDirs(path);
        }
    }
    closedir(d);
}

int isDirectory(char* path){
    struct stat statbuf;
    stat(path, &statbuf);
    return S_ISDIR(statbuf.st_mode);
}

int isReg(char* path){
    struct stat statbuf;
    stat(path, &statbuf);
    return S_ISREG(statbuf.st_mode);
}

int fileType(char* path){
    struct stat s; 
    if(stat(path, &s) == 0){
        if(s.st_mode & __S_IFDIR){
            return 1; //IS DIRECTORY
        }else if (s.st_mode & __S_IFREG){
            return 2; //IS REG FILE
        }else{
            return 0; //SOMETHING ELSE
        }
    }else{
        //printf("*** fileType ERROR ***\n");
        return -1;
    }
}

void transferDataToFile(char* read_path, char* write_path){
    int fd_read = open(read_path, O_RDONLY);
    int fd_write = open(write_path, O_RDWR | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

    if(fd_write == -1 && errno == 2){
        //perror("Program");
        creat(write_path, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        //printf("Creating File...\n");
        fd_write = open(write_path, O_RDWR | O_APPEND);
    }

    if (fd_read == -1 || fd_write == -1) { 
        printf("Error Number %d\n", errno);  
        perror("Program");                  
    } 

    char* read_buf;
    
    int r;
    int w;
    int writeAmount;

    do{
        writeAmount = 1;
        read_buf = (char*) malloc(4*sizeof(char));
        r = read(fd_read, read_buf, 1);

        //SPECIAL CHARACTERS: '\n' '\t' '\v' '\r' 
        if(strcmp(read_buf, "\n") == 0){
            strcat(read_buf, "~n~");
            writeAmount = 4;
        }
        if(strcmp(read_buf, "\t") == 0){
            strcat(read_buf, "~t~");
            writeAmount = 4;
        }
        if(strcmp(read_buf, "\v") == 0){
            strcat(read_buf, "~v~");
            writeAmount = 4;
        }
        if(strcmp(read_buf, "\r") == 0){
            strcat(read_buf, "~r~");
            writeAmount = 4;
        }

        //strcpy(read_buf, write_buf);
        if(r != 0){
            w = write(fd_write, read_buf, writeAmount);
        }
        
        if(w == -1){
            printf("Error Number %d\n", errno);  
            perror("Program");
            exit(1);
        }
        free(read_buf);
    }while(r != 0);

    close(fd_read);
    close(fd_write);
}

int find_and_transfer(char* target){
    DIR *d;
    struct dirent *dir;
    d = opendir(".");
    if(d){
        while((dir = readdir(d)) != NULL){
            int type = isReg(dir->d_name);
            if(type != 0){
                //printf("%s\n", dir->d_name);

                if(strcmp(dir->d_name, target) == 0){
                    //printf("Tranferring %s data...\n", dir->d_name);
                    transferDataToFile(dir->d_name, "write_temp.txt");
                    return 0;
                }

            }
        }
        closedir(d);
    }else{
        printf("Error Number %d\n", errno);  
            perror("Program");
            exit(1);
    }

    return -1;
    
}

int rec_transfer(char* bpath, char* masterPath){ 
    
    char path[1000];
    struct dirent* dp;
    DIR *d = opendir(bpath);

    if(!d){
        return -1;
    }

    while((dp = readdir(d)) != NULL){
        if(strcmp(dp->d_name,"..") != 0 && strcmp(dp->d_name,".") != 0){
            
            strcpy(path,bpath);
            strcat(path,"/");
            strcat(path, dp->d_name);
            //printf("%s\n", path);

            int type = isReg(path);
            if(type != 0){
                //printf("%s\n", path);
                

                char* writePath = malloc(100 * sizeof(char));
                strcpy(writePath, masterPath);
                strcat(writePath, "/write_temp.txt");
                //strcpy(writePath, bpath);
                //printf("write path: %s\n", writePath);
                if(strcmp(path, writePath) != 0){
                    //printf("Tranferring %s data...\n", path);
                    transferDataToFile(path, writePath);
                }
                
               free(writePath);
            }

            rec_transfer(path, masterPath);
        }
    }
    closedir(d);
}

WordOcc* createOcc(int freq, char* word){
    WordOcc *occ = (WordOcc*) malloc(sizeof(WordOcc));

    if(occ == NULL){
        printf("createOcc Memory Error!");
        return;
    }

    occ->freq = freq;
    occ->word = word;

    return occ;
}

void printOcc(WordOcc* occ){
    if(occ == NULL){
        printf("Occ is NULL\n");
        return;
    }
    printf("(%d, %s)\n", occ->freq, occ->word);
}

int getFreq_occ(WordOcc* occ){
    return occ->freq;
}

char* getWord_occ(WordOcc* occ){
    return occ->word;
}

int get_max(int x, int y){
    return(x > y)? x : y;
}

int get_height(Node* node){
    if(node == NULL){
        return 0;
    }
    return node->height;
}

Node* newNode(WordOcc* key){
    Node* node = (Node*) malloc(sizeof(Node));
    node->key = key;
    node->left = NULL;
    node->right = NULL;
    node->height = 1;
    return node;
}

int avl_get_key_val(Node* node){
    return getFreq_occ(node->key);
}

char* avl_get_key_word(Node* node){
    return getWord_occ(node->key);
}

Node* rotate_right(Node* y){
    Node* x = y->left;
    Node* tree2 = x->right;
    x->right = y;
    y->left = tree2;
    y->height = get_max(get_height(y->left), get_height(y->right)) + 1;
    x->height = get_max(get_height(x->left), get_height(x->right)) + 1;
    return x;
}

Node* rotate_left(Node* x){
    Node* y = x->right;
    Node* tree2 = y->left;
    y->left = x;
    x->right = tree2;
    x->height = get_max(get_height(x->left), get_height(x->right)) + 1;
    y->height = get_max(get_height(y->left), get_height(y->right)) + 1;
    return y;
}

int get_balance_factor(Node* node){
    if(node == NULL){
        return 0;
    }
    return get_height(node->left) - get_height(node->right);
}

Node* insert(Node* node, WordOcc* key){//COMPARE ASCII --> if equal increment frequency
    if(node == NULL){
        return(newNode(key));
    }

    if(strcmp(key->word, node->key->word) < 0){
        node->left = insert(node->left, key);
    }else if(strcmp(key->word, node->key->word) > 0){
        node->right = insert(node->right, key);
    }else if(strcmp(key->word, node->key->word) == 0){
        node->key->freq++;
        return node;
    }

    node->height = 1 + get_max(get_height(node->left), get_height(node->right));

    int bf = get_balance_factor(node);

    if(bf > 1 && strcmp(key->word, node->left->key->word) < 0){
        return rotate_right(node);
    }
    if(bf < -1 && strcmp(key->word, node->right->key->word) > 0){
        return rotate_left(node);
    }
    if(bf > 1 && strcmp(key->word, node->left->key->word) > 0){
        node->left = rotate_left(node->left);
        return rotate_right(node);
    }
    if(bf < -1 && strcmp(key->word, node->right->key->word) < 0){
        node->right = rotate_right(node->right);
        return rotate_left(node);
    }
    return node;
}

Node* get_smallest_node(Node* node){
    Node* crnt = node;
    if(crnt == NULL){
        return crnt;
    }
    while(crnt->left != NULL){
        crnt = crnt->left;
    }
    return crnt;
}

Node* avl_remove(Node* root, WordOcc* key){
    if(root == NULL){
        return root;
    }
    printOcc(key);

    if(strcmp(key->word, root->key->word) < 0){
        root->left = avl_remove(root->left, key);
    }else if(strcmp(key->word, root->key->word) > 0){
        root->right = avl_remove(root->right, key);
    }else if(strcmp(key->word, root->key->word) == 0){
        if(root->left == NULL || root->right == NULL){
            Node* tmp = root->left ? root->right : root->right;
            if(tmp == NULL){
                tmp = root;
                root = NULL;
            }else{
                *root = *tmp;
            }
            free(tmp);
        }else{
            Node* tmp = get_smallest_node(root->right);
            //strcpy(root->key->word, tmp->key->word);
            WordOcc* rootKey = root->key;
           // printOcc(rootKey);
            WordOcc* tmpKey = tmp->key;
            //printOcc(tmpKey);
            *rootKey = *tmpKey;
            //printOcc(rootKey);

            root->right = avl_remove(root->right, tmp->key);
        }
    }

    if(root == NULL){
        return root;
    }

    root->height = 1 + get_max(get_height(root->left), get_height(root->right));

    int bf = get_balance_factor(root);

    if(bf > 1 && get_balance_factor(root->left) >= 0){
        return rotate_right(root);
    }
    if(bf > 1 && get_balance_factor(root->left) < 0){
        root->left = rotate_left(root->left);
        return rotate_right(root);
    }
    if(bf < -1 && get_balance_factor(root->right) <= 0){
        return rotate_left(root);
    }
    if(bf < -1 && get_balance_factor(root->right) > 0 ){
        root->right = rotate_right(root->right);
        return rotate_left(root);
    }

    return root;
}

Node* remove_root(Node* root){
    if(root == NULL || root->key == NULL){
        //printf("******Tree is empty********\n");
        return NULL;
    }
    //printOcc(root->key);
    //printOcc(root->key);
    Node* rootCopy = newNode(root->key);
    Node* output = rootCopy;
    //printOcc(output->key);

    if(root->right == NULL && root->left == NULL){
        //printf("POPPED: ");
        //printOcc(output->key);
        //printf("\n");
        //root = NULL;
        free(root);
        return output;
    }

    if(root->right != NULL){
        //Node* temp = get_smallest_node(root->right);
        Node* parent = root->right;
        Node* crnt = parent;
        if(parent->left != NULL){
            crnt = parent->left;
        }
        

        while(crnt != NULL && crnt->left != NULL){
            crnt = crnt->left;
            parent = parent->left;
        }
        //printf("SUBTREE(1):\n");
        //printTree(parent,10);
        //printOcc(parent->key);
        root->key = crnt->key;
        //printf("Removing: ");
        //printOcc(crnt->key);
        //printf("\n");


        if(strcmp(crnt->key->word, parent->key->word) != 0){
            parent->left = crnt->right;
        }else if(strcmp(crnt->key->word, parent->key->word) == 0){
            //printf("\nParent is same as crnt\n");
            if(parent->left == NULL && parent->right == NULL){
                //printf("IS LEAF NODE\n");
                root->right = NULL;
                //printf("POPPED: ");
                //printOcc(output->key);
                //printf("\n");
                return output;
            }
            //root->key = crnt->key;
            //remove_root(parent);
            //printf("New parent:");
            parent = parent->right;
            //printOcc(parent->key);
            //printf("\n");
            //printTree(parent,10);
            root->right = parent;
        }
        
        //free(crnt);
        
        //printTree(temp, 10);
    }else if(root->left != NULL){
        
        
        //printf("SUBTREE(2):\n");
        //printTree(root->left,10); 
        *root = *root->left;
        //printf("\n\n\n");
        
        //printTree(root,10); 
    }

    

    //printf("POPPED: ");
    //printOcc(output->key);
    //printf("\n");
    return output;
}

void print_preOrder(Node* root){
    if(root != NULL){
        printOcc(root->key);
        print_preOrder(root->left);
        print_preOrder(root->right);
    }
}

MinHeapNode* newHeapNode(WordOcc* data){
    MinHeapNode* temp = (MinHeapNode*) malloc(sizeof(MinHeapNode));
    temp->left = NULL;
    temp->right = NULL;
    temp->data = data;
    return temp;
}

MinHeap* createMinHeap(int capacity){
    MinHeap* minHeap = (MinHeap*) malloc(sizeof(MinHeap));

    minHeap->size = 0;
    minHeap->capacity = capacity;
    minHeap->arr = (MinHeapNode**) malloc(minHeap->capacity * sizeof(MinHeapNode*));
    return minHeap;
}

void switchHeapNodes(MinHeapNode** x, MinHeapNode** y){
    MinHeapNode* t = *x;
    *x = *y;
    *y = t;
}

void heapify(MinHeap* heap, int index){
    if(heap == NULL || heap->arr == NULL){
        //printf("heap is NULL\n");
        return;
    }
    int smallest = index;
    int left = 2 * index+1;
    int right = 2 * index+2;

    if(smallest > heap->size || left > heap->size || right > heap->size){
        //printf("** Trying to access index larger than heap size\n");
        return;
    }

    if(heap->arr[left] == NULL || heap->arr[right] == NULL || heap->arr[smallest] == NULL){
        //printf("Data of arr is null\n");
        return;
    }

    //printf("%d\n", heap->size);
    //printf("%d\n", heap->arr[left]->data->freq);
    //printf("%d\n", heap->arr[smallest]->data->freq);

    if(left < heap->size && heap->arr[left]->data->freq < heap->arr[smallest]->data->freq){
        smallest = left;
    } 
    if(right < heap->size && heap->arr[right]->data->freq < heap->arr[smallest]->data->freq){
        smallest = right;
    }
    if(smallest != index){
        switchHeapNodes(&heap->arr[smallest], &heap->arr[index]);
        heapify(heap, smallest);
    }
}

int isOne(MinHeap* heap){
    return(heap->size == 1);
}

void heap_insert(MinHeap* heap, MinHeapNode* node){
    ++heap->size;
    int i = heap->size - 1;
    while(i && node->data->freq < heap->arr[(i-1)/2]->data->freq){
        heap->arr[i] = heap->arr[(i-1)/2];
        i = (i-1)/2;
    }
    heap->arr[i] = node;
}

void build_heap(MinHeap* heap){
    int n = heap->size -1;
    int i;
    for(i = (n-1)/2; i >= 0; --i){
        heapify(heap, i);
    }
}

MinHeapNode* heap_get_min(MinHeap* heap){
    MinHeapNode* temp = heap->arr[0];
    heap->arr[0] = heap->arr[heap->size -1];
    --heap->size;
    heapify(heap, 0);
    return temp;

}

int isLeaf(MinHeapNode* root){
    return !(root->left) && !(root->right);
}

void printTree(Node* root, int space){
    if(root == NULL){
        return;
    }

    space += 10;

    printTree(root->right, space);

    printf("\n");
    int i;
    for(i = 10; i < space; i++){
        printf(" ");
    }
    printOcc(root->key);
    printf("\n");

    printTree(root->left, space);
}

MinHeap* makeMinHeap(Node* wordTree, int size){
    if(wordTree == NULL || wordTree->key == NULL){

    }

    MinHeap* heap = createMinHeap(size);
    int i;
    for(i = 0; i< size; ++i){
        Node* temp = (Node*) malloc(sizeof(Node));
        if(wordTree != NULL && wordTree->key != NULL){
            temp = remove_root(wordTree);
        }
        
        //heap->arr[i] = newHeapNode(temp->key);
        if(temp != NULL){
            //printOcc(temp->key);
            heap->arr[i] = newHeapNode(temp->key);
        }else{
            break;
        }
        
    }
    heap->size = size;
    build_heap(heap);
    return heap;
}

MinHeapNode* makeHuffmanTree(Node* wordTree, int size){
    MinHeapNode* left;
    MinHeapNode* right;
    MinHeapNode* top;

    MinHeap* heap = makeMinHeap(wordTree, size);

    while(!isOne(heap)){
        left = heap_get_min(heap);
        right = heap_get_min(heap);

        WordOcc* internal = createOcc(left->data->freq+right->data->freq, "");
        top = newHeapNode(internal);
        top->left = left;
        top->right = right;
        heap_insert(heap, top);
    }

    return heap_get_min(heap);
}

void printArray(int arr[], int n){
    int i;
    for(i = 0; i < n; ++i){
        printf("%d", arr[i]);
    }
    printf("\n");
    
}

void printCodes(MinHeapNode* root, int arr[], int top){ //Loads Words and codes into HuffmanCodebook.txt (also prints)

    if(root->left){
        arr[top] = 0;
        printCodes(root->left, arr, top+1);
    }

    if(root->right){
        arr[top] = 1;
        printCodes(root->right, arr, top+1);
    }

    int fd_write = open("HuffmanCodebook.txt", O_RDWR | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

    if(fd_write == -1 && errno == 2){
        //perror("Program");
        creat("HuffmanCodebook.txt", S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        //printf("Creating File...\n");
        fd_write = open("HuffmanCodebook.txt", O_RDWR | O_APPEND);
    }

    if (fd_write == -1) { 
        printf("(PrintCodes) Error Number %d\n", errno);  
        perror("Program");                  
    }

    if(isLeaf(root)){
        //printOcc(root->data);
        //printArray(arr, top);
        char* code = intArray_to_string(arr, top);
        char* word = root->data->word;
        //printf("%s:\t%s\n", word, code);
        size_t word_len = strlen(word);
        size_t code_len = strlen(code);
        size_t max_len = word_len + code_len;
        char* write_buf = (char*) malloc(max_len + 3);
        memcpy(write_buf, code, code_len);
        write_buf[code_len] = '\t';
        //write_buf[code_len + 1] = '\0';
        strcat(write_buf, word);
        write_buf[max_len + 1] = '\n';
        write_buf[max_len + 2] = '\0';


        //strcat(write_buf, "\t\0");
        //printf("Write_buff: %s", write_buf);
        

        int w;
        int writeAmount = 1;

        w = write(fd_write, write_buf, strlen(write_buf));
        if(w == -1){
            printf("Error Number %d\n", errno);  
            perror("Program");
            exit(1);
        }
        free(write_buf);
        //printf("String from code array: %s\n", str);
    }
}

void huffmanCodes(Node* wordTree, int size){
    MinHeapNode* root = makeHuffmanTree(wordTree, size);

    int height = getTreeHeight_Huff(root);

    int arr[height]; // MAXIMUM HIGHT OF TREE
    int top = 0;
    printCodes(root, arr,top);
}

int getTreeHeight(Node* root){ //Returns the MAXIMUM tree height -- There is a degree of imprecision
    int count_left = 0;
    int count_right = 0;
    if(root == NULL || root->key == NULL){
        return -1;
    }

    //int result = 0;
    Node* crnt = root;

    //Left Size of tree:
    while(crnt->left != NULL && crnt->key != NULL){
        crnt = crnt->left;
        count_left++;
    }

    //Right side of tree:
    crnt = root;
    while(crnt->right != NULL && crnt->key != NULL){
        crnt = crnt->right;
        count_right++;
    }

    return (get_max(count_left, count_right) +1 );
}

int getTreeHeight_Huff(MinHeapNode* root){
    int count_left = 0;
    int count_right = 0;
    if(root == NULL || root->data == NULL){
        return -1;
    }

    //Left side of tree:    
    MinHeapNode* crnt = root;
    while(crnt->left != NULL && crnt->data != NULL){
        crnt = crnt->left;
        count_left++;
    }

    //Right side of tree:
    crnt = root;
    while(crnt->right != NULL && crnt->data != NULL){
        crnt = crnt->right;
        count_right++;
    }

    return get_max(count_left, count_right);

}

int getMaxNumElements(int height){
    //int res = (int) pow(2, height);
    //return res;
    return (1 << (height +1))-1;
}

char* intArray_to_string(int arr[], int size){
    char* str = (char*) malloc(size * sizeof(int));
    int i;

    for(i = 0; i < size; ++i){
        int digit = arr[i];
        char charDig = '0' + digit;
        str[i] = charDig;
    }
    str[size+1] = '\0';

    return str;
}

//Transfer file contents to write_temp file
//Tokenize and load tokens into avl tree
//build Huffman codes
//Outputs the HuffmanCodebook file
void buildCodeBook(char* flags, char* filename){
    if(flags[0] != '-'){
        printf("Error: Flags must start with '-'\n");
        exit(1);
    }
    if(flags[1] != 'R' && flags[1] != 'b'){
        printf("Error: Invalid Flag(s)\n");
        exit(1);
    }

    int mode;

    if(flags[1] == 'R'){
        printf("* Recursive Mode...\n");
        mode = 0;

        if(isReg(filename)){
            printf("Error: Use Directory path for recursive mode...\n");
            exit(1);
        }

        rec_transfer(filename, filename);
    }else{
        printf("* NON-Recursive mode...\n");
        mode = 1;

        if(isDirectory(filename)){
            printf("Error: Use SINGLE filepath for regular mode...\n");
            exit(1);
        }

        transferDataToFile(filename, "write_temp.txt");
    }

    //Scan write_temp.txt and tokenize/Load words into avl tree.
    int fd_read;
    if(mode == 0){ // Recursive mode
        char* directory = (char*) malloc(128 * sizeof(char));
        strcpy(directory, filename);
        strcat(directory, "/write_temp.txt");
        fd_read = open(directory, O_RDONLY);

        free(directory);
    }else if(mode == 1){ //Regular mode
        fd_read = open("write_temp.txt", O_RDONLY);
    }

    //printf("Opening write_temp.txt...\n");
    if (fd_read == -1) { 
        printf("Error Number %d\n", errno);  
        perror("Program");        
        exit(1);          
    } 

    char* read_buf;
    char* word = (char*) malloc(128 * sizeof(char));
    memcpy(word, "", 1);
    
    int r;
    int loop_max = 0;
    Node* root = NULL;

    do{
        read_buf = (char*) malloc(sizeof(char));
        r = read(fd_read, read_buf, 1);
        strcat(word, read_buf);

        if(r == 0){
            //printf("End of file...\n");
        }

        if(strcmp(read_buf, " ") == 0){
            word[strlen(word)-1 ] = '\0';
            //printf("Scanned Word(1): %s\n", word);
            //r = read(fd_read, read_buf, 1);
            char* final = malloc(128 * sizeof(char));
            strcpy(final, word);
            root = insert(root, createOcc(1, final));
            root = insert(root, createOcc(1, " "));
            memcpy(word, "", 1);
        }

        if(strcmp(read_buf, "~") == 0){
            char* newLine = (char*) malloc(4*sizeof(char));
            memcpy(newLine, "~", 1);
            r = read(fd_read, read_buf, 1);
            if(r > 0 && strcmp(read_buf, "n") == 0){
                strcat(newLine, "n~");
                //printf("Special Character found: %s\n", newLine);
                root = insert(root, createOcc(1, newLine));;
                r = read(fd_read, read_buf, 1);
            }
            //char c = word[strlen(word)-1];
            //printf("char at end of word: %c\n", c);
            word[strlen(word)-2 ] = '\0';
            //printf("Scanned word(2): %s\n", word);
            char* final = malloc(128 * sizeof(char));
            strcpy(final, word);
            root = insert(root, createOcc(1, final));
            memcpy(word, "", 1);
        }



        //root = insert(root, createOcc(1, word));

        //free(read_buf);
        //break;
    }while(r != 0 /* && loop_max < 200 */);

    //printf("Scanned Word: %s\n", word);
    root = insert(root, createOcc(1, word));

    int height = getTreeHeight(root);
    //printf("Tree height = %d\n", height);
    //printf("Max # of elements: %d\n", getMaxNumElements(height));

    //int size = getMaxNumElements(height);
    int size = count_postOrder(root);

    //printf("size of tree: %d\n", size);

    //printTree(root, 10);

    if(root != NULL){
        /* MinHeap* heap = makeMinHeap(root, size);
        int i =0;
        printf("Contents of minheap: \n");
        for(i = 0; i < size; i++){

            printOcc(heap->arr[i]->data);
        } */
        huffmanCodes(root, size);
    }
    

    close(fd_read);

    if(mode == 0){ // Recursive mode
        char* directory = (char*) malloc(128 * sizeof(char));
        strcpy(directory, filename);
        strcat(directory, "/write_temp.txt");
        remove(directory);
        free(directory);
    }else if(mode == 1){ //Regular mode
        remove("write_temp.txt");
    }

    printf("Codebook Generated...\n");
    
    //free(word);
    
}

int count_postOrder(Node* root){

    int res = 1;

    if(root == NULL){
        return 0;
    }else{
        res += count_postOrder(root->left);
        res += count_postOrder(root->right);
        return res;
    }



    /* if(root == NULL){
        return 0;
    }

    int res =0;
    if(root->left && root->right){
        res++;
    }
    res += (count_postOrder(root->left) + count_postOrder(root->right));
    return res; */
}
