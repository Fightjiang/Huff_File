#include <iostream> 
#include <fstream>
#include <string> 
#include <unordered_map> 
#include <queue>
#include <algorithm> 

using namespace std ; 

#define		TARGET_FILE_EXT		".dhuf"

// Huffman 树节点
struct node {
    string s ; 
    int weight ; 
    node *left ; 
    node *right ; 
    node() : s("") , weight(0) , left(NULL) , right(NULL) {} ; 
};

typedef struct node* HuffmanTree , *HuffmanNode ; 

unordered_map<string, string> CodeTable ; // 哈夫曼编码表


void Read_CodeTable(char *fileName) {
    ifstream infile ; 
    infile.open(fileName) ; 
    if(!infile.is_open()) {
        cout << "文件 " << fileName << " 打开失败!" << endl;
		cout << "请检查: 1.文件是否存在 2. 路径是否正确 3.是否无权限访问该文件 / 文件夹" << endl;
		exit(-1); //终止程序，抛出异常值
    }

    string str ; char ch ; 
    while(infile.peek() != EOF) {
        char ch = infile.get() ;  str = ch ; 
        if(ch > 127 || ch < 0) { // 处理中文
            ch = infile.get() ; str += ch ; 
            ch = infile.get() ; str += ch ;
        }
        string coding ; infile >> coding ; infile.get() ; // 读掉换行符 
        CodeTable[coding] = str ; 
    }

    infile.close() ; 
    //for(auto it : CodeTable)  cout<<it.first <<" " << it.second<<endl ; 

}

HuffmanNode Rebuild_Huffman() {
    HuffmanNode current ,  root = new node() ; 

    for(auto it : CodeTable) {
        int size = it.first.size() ;
        current = root ;     
        for(int i = 0 ; i < size ; ++i) {
            HuffmanNode tmp = new node() ; 
            if(it.first[i] == '0') {
                if(current->left == NULL) current->left = tmp ; // 存在内存泄漏
                current = current->left ; 
            } else {
                if(current->right == NULL) current->right = tmp ; 
                current = current->right ; 
            }
            if(i == size - 1) current->s = it.second ; 
        }
    } 

    return root ; 
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

/* 生成 string 的过程太慢了，不能接受呀
void Decryption(char *fileName , HuffmanNode root) {
    fstream infile , outfile ; 
    infile.open(fileName , ios::in | ios:: binary) ; 
    outfile.open(getTargetFileName(fileName , TARGET_FILE_EXT)  , ios::out) ; 

    unsigned char ch ; int last_bit = -1 ; string ZeroOne_s = ""; int C_Bit = 8 ; 
    //依次读取二进制文件中的unsigned char值读入到变量i,并输出
    while(infile.read((char*)&ch , sizeof(ch))) {
        if(last_bit == -1) {last_bit = ch ; continue ; }
        
        //已经是最后一行了，根据第一个余数进行修改
        if(infile.peek() == EOF) {C_Bit = (last_bit == 0) ? 8 : last_bit ;}
        
        for(int i = 0 ; i < C_Bit ; ++i) {
            // 位运算，两次取非，我代码写的真好
            ZeroOne_s = ZeroOne_s + (char)(!!(ch & (1 << (7 ^ i))) + '0') ; 
        }
        //cout<<infile.peek()<<endl ; 
    }
    //cout<< ZeroOne_s <<endl ; 
    size_t i = 0 , size = ZeroOne_s.size() ; 
    while( i < size) {
        int cnt = 0 ; HuffmanNode cur = root ; 
        while(cur->left != NULL && cur->right != NULL) {
            if(ZeroOne_s[i++] == '0') cur = cur->left ; 
            else cur = cur->right ; 
        }
       // cout<<i<<endl ; 
        outfile<<cur->s ; 
    }
    infile.close() ; outfile.close() ; 
}
*/

void Decryption(char *fileName , HuffmanNode root) {
    fstream infile , outfile ; 
    infile.open(fileName , ios::in | ios:: binary) ; 
    outfile.open(getTargetFileName(fileName , TARGET_FILE_EXT)  , ios::out) ; 

    unsigned char ch ; int last_bit = -1 ; string ZeroOne_s = ""; int C_Bit = 8 ; 
    //依次读取二进制文件中的unsigned char值读入到变量i,并输出
    while(infile.read((char*)&ch , sizeof(ch))) {
        if(last_bit == -1) {last_bit = ch ; continue ; }
        
        //已经是最后一行了，根据第一个余数进行修改
        if(infile.peek() == EOF) {C_Bit = (last_bit == 0) ? 8 : last_bit ;}
        
        for(int i = 0 ; i < C_Bit ; ++i) {
            // 位运算，两次取非，我代码写的真好
            ZeroOne_s = ZeroOne_s + (char)(!!(ch & (1 << (7 ^ i))) + '0') ; 
            
            auto iter =  CodeTable.find(ZeroOne_s) ; 
            // 每个字符对应的 01 编码是独一无二的，直接查表写入；
            if(iter != CodeTable.end() ) {
                outfile << iter->second ; 
                ZeroOne_s = "" ; 
            }
        }
    }
    
    infile.close() ; outfile.close() ; 
}


int main(int argc , char *args[]) {

    if(argc != 3){
		puts("用法: hufDecoding <源文件名> <译码表>");
		return 0;
	}

    cout << "正在根据译码表构造 Huffman 树..." << endl;
    Read_CodeTable(args[2]) ; 
    
    HuffmanTree root = Rebuild_Huffman() ; 
    cout << "Huffman 树重构完毕，正在对 Huf编码文件进行解码..." << endl;
    
    Decryption(args[1] , root); //对加密文件进行解码
	cout << "解码成功 !" << endl;

    return 0 ; 
}