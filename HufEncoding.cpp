#include <iostream> 
#include <fstream>
#include <string> 
#include <map> 
#include <queue>
#include <algorithm> 

using namespace std ; 

#define		TARGET_FILE_EXT		".huffman"
#define		TARGETCODE_FILE_EXT		".huffman.code"

// Huffman 树节点
struct node {
    string s ; 
    int weight ; 
    node *left ; 
    node *right ; 
    node() : s("") , weight(0) , left(NULL) , right(NULL) {} ; 
};

//自定义结点优先级（用于优先队列）
struct cmp {
    bool operator() (const node* a , const node* b) {
        return a->weight > b->weight ; // 小顶堆
    }
};

// 哈夫曼树指针，哈夫曼树结点
typedef struct node* HuffmanTree , *HuffmanNode ; 

map<string , int > FrequencyTable ; // 字符频率表
map<string, string> CodeTable ; // 哈夫曼编码表

//优先队列，取权值最小结点的堆
priority_queue<HuffmanNode , vector<HuffmanNode> ,cmp> Q ; 

// 存储 源文件 ， 哈夫曼 01 编码，解码文件的字符串
//string Source_s , ZeroOne_s , Decoding_s ; 

const int C_BIt = 8 ; // C_Bit 位压缩 
size_t ZeroOne_size = 0 ; // 统计 01 串的总长度，方便进行最后不足 8 位时的处理，并将余数存入加密文件第一位。


// 统计每个字符出现的频率
void getFreq(char *fileName) {
    ifstream infile ; 
    infile.open(fileName) ; 
    if(!infile.is_open()) {
        cout << "文件 " << fileName << " 打开失败!" << endl;
		cout << "请检查: 1.文件是否存在 2. 路径是否正确 3.是否无权限访问该文件 / 文件夹" << endl;
		exit(-1); //终止程序，抛出异常值
    }
    
    string s ; char ch ; 
    while(infile.peek() != EOF) {
        ch = infile.get() ; s = ch ; 
        if(ch > 127 || ch < 0) { // utf-8 中文读三个字符
            ch = infile.get() ; s = s + ch ; 
            ch = infile.get() ; s = s + ch ; 
        }
        ++FrequencyTable[s] ; 
    }
    infile.close() ;
    for(auto it : FrequencyTable){
        HuffmanNode tmp = new node() ;
        tmp->s = it.first ; tmp->weight = it.second ; tmp->left = NULL ; tmp->right = NULL ; 
        Q.push(tmp) ; 
    } 
}

//根据小顶堆，构造哈夫曼树
HuffmanTree Build_Huffman() {
    while(Q.size() > 1) {
        HuffmanNode tmp = new node() ; 
        tmp->left = Q.top() ; Q.pop() ; 
        tmp->right = Q.top() ; Q.pop() ; 
        tmp->weight = tmp->left->weight + tmp->right->weight ; 
        Q.push(tmp) ; 
    }
    return Q.top() ; // 返回哈夫曼树的 root 
}

// 根据生成的哈弗曼树，生成每个字符对应的 01 编码
void makeHuffCode(HuffmanTree T , string s) {
    if(T->left == NULL && T->right == NULL) {
        CodeTable[T->s] = s ; 
        ZeroOne_size += s.size()*FrequencyTable[T->s] ; 
        return ; 
    }
    makeHuffCode(T->left , s + '0') ; 
    makeHuffCode(T->right , s + '1') ;  
}

// 建立目标文件名
string getTargetFileName(char *sourceFileName , string tail){
	string targetFileName = "" ; 
	for(int i = 0; sourceFileName[i] != '\0'; i++){
		if(sourceFileName[i] == '.'){
			break;
		}
        targetFileName += sourceFileName[i] ; 
	}
	// 重新添加文件名后缀为 ".Mechuf"
	return targetFileName + tail ;  
}

// 将全部的 01 编码串，按 bit 压缩成 huffman 加密文件
void Save_Encrypted(char *fileName) {
    
    string targetFile = getTargetFileName(fileName , TARGET_FILE_EXT) ; 
    fstream outfile , infile; 
    outfile.open(targetFile , ios::binary | ios::out) ; 
    infile.open(fileName , ios::in) ; 

    unsigned char bitIndex = 0 , byte = 0; 
    string str ; char ch ; 

    /*将余数存入加密文件第一位*/
    unsigned char last_bit = (unsigned char)(ZeroOne_size % C_BIt); 
    outfile.write((char*)&last_bit, sizeof(last_bit));
    
    while(infile.peek() != EOF) {
        ch = infile.get() ; str = ch ; 
        if(ch > 127 || ch < 0) { // utf-8 中文读三个字符
            ch = infile.get() ; str = str + ch ; 
            ch = infile.get() ; str = str + ch ; 
        }
        for(auto code : CodeTable[str]) {

            if(code == '1') {
                byte = byte | (1 << (7 ^ (bitIndex++))) ; //设置这个字节的指定位为1 , 巧妙 异或 ， 从高位开始
            }else {
                byte = byte & ~(1 << (7 ^ (bitIndex++))) ; //设置这个字节的指定位为0
            }
            if(bitIndex == 8) {
                outfile.write((char *)&byte , sizeof(byte)) ; 
                bitIndex = 0 ; byte = 0 ; 
            }
        }
    }
    if(bitIndex) {
         outfile.write((char *)&byte , sizeof(byte)) ; 
    }
}

 //保存译码表文件 ， 方便进行解码
void Write_CodeTable(char *fileName) {
    string target_CodeFile = getTargetFileName(fileName , TARGETCODE_FILE_EXT) ; 
    ofstream outfile; 
    outfile.open(target_CodeFile) ; 

	for(auto it : CodeTable) 
        outfile<< it.first + " " + it.second  <<endl ;
	
    outfile.close() ; 
}

//获取文件大小
size_t Get_FileSize(const string& file_name) {
	ifstream in(file_name.c_str());
	in.seekg(0, ios::end);
	size_t size = in.tellg();
	in.close();
	return size; //单位是：byte
}


int main(int argc , char *args[]) {

    if(argc <= 1 || argc > 3){
		puts("用法: textFileHuff <源文件名> [目标文件名]");
		return 0;
	}

    cout << "打开原始文件成功，正在根据字符出现频率构造Huffman树..." << endl;
    getFreq(args[1]) ; 
    
    HuffmanTree root = Build_Huffman() ; 
    cout << "Huffman 树构造成功，正在进行 Huffman 编码..." << endl;

    makeHuffCode(root , "") ; 
    cout << "Huffman 编码成功， 正在进行 Huffman 编码压缩" << endl ;
	Save_Encrypted(args[1]); //压缩并保存加密文件

    cout << "压缩文件保存完成 , 正在保存译码表文件 \n" << endl;
    Write_CodeTable(args[1]); //输出译码表文件
	cout << "全部操作完成！" << endl;
    delete root ; 

    return 0 ; 
}