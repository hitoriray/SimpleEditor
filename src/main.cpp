#include <iostream>
#include <algorithm>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <unordered_map>


/* 宏定义，用于打印各种不同等级的信息，便于调试 */
#define INFO(message) std::cout << Hazel::GREEN << message << Hazel::RESET << std::endl;
#define WARNINGS(message) std::cout << Hazel::YELLOW << message << Hazel::RESET << std::endl;
#define TRACE(message) std::cout << Hazel::MAGENTA << message << Hazel::RESET << std::endl;
#define ERROR(message)\
        std::cerr << Hazel::RED << "Error: " << Hazel::YELLOW << __FILE__ << Hazel::RESET\
            << " : in function " << Hazel::CYAN <<  __func__ << Hazel::RESET\
            << " at line " << Hazel::MAGENTA << __LINE__ << Hazel::RESET << std::endl\
            << "        Compiled on " << __DATE__\
            << " at " << __TIME__ << std::endl\
            << "        " << Hazel::RED << message << Hazel::RESET << std::endl;


/* 定义一些要用到的常量 */
namespace Hazel
{
    // 颜色变量
    const char* RESET = "\033[0m";
    const char* BLACK = "\033[30m";
    const char* RED = "\033[31m";
    const char* GREEN = "\033[32m";
    const char* YELLOW = "\033[33m";
    const char* BLUE = "\033[34m";
    const char* MAGENTA = "\033[35m";
    const char* CYAN = "\033[36m";
    // 一些大小常量
    const int MAXACTIVELEN = 100;  // 活区的最大长度
    const int MAXLINELEN = 321;  // 每一行的最大长度
    const int MAXBLOCKLEN = 81;  // 每一个行块的最大长度
    const int MAXFILENAMELEN = 100;  // 文件名的最大长度
    // 字符串匹配算法的选择
    const int KMP = 1;
    const int TRIE = 2;
    const int BF = 3;
};


/* 结构体的定义 */

// 存储每一行的文本，链式存储
typedef struct LineBlock {
    char data[Hazel::MAXBLOCKLEN];  // 行中每一块的字符内容
    LineBlock* next;  // 指向下一个行块
    // 默认构造
    LineBlock(const char* text, LineBlock* ne = nullptr)
    {
        int length = strlen(text);
        memcpy(data, text, std::min(length, Hazel::MAXLINELEN - 1));  // 把text的内容拷贝给data
        data[length] = '\0';
        next = ne;
    }
} LineBlock;
// 存储活区中的每一行，链式存储
typedef struct Line {
    int line_no;  // 行号
    LineBlock* content;  // 指向行内容
    Line* next;  // 指向下一行
    Line(int number, LineBlock* line_block, Line* ne = nullptr) : line_no(number), content(line_block), next(ne) {}
} Line, *ActiveArea;
// 存储活区中匹配到的字符串所处的位置(row,col)
typedef struct Position {
    int row, col;
} Pos;
// Trie节点
struct TrieNode {
    bool isEnd;  // 标记是否是单词的结尾
    std::unordered_map<char, TrieNode*> son;  // 子节点集合
    TrieNode() : isEnd(false) {}
};
// Trie树
class Trie {
public:
    // 默认构造
    Trie() : root(new TrieNode()) {}
    // 插入字符串
    void insert(const char* word)
    {
        TrieNode* node = root;
        while (*word)
        {
            if (node->son.find(*word) == node->son.end())
                node->son[*word] = new TrieNode();
            node = node->son[*word];
            word ++ ;
        }
        node->isEnd = true;
    }
    // 搜索字符串
    bool search(const char* src, const char* match_str, int pos[], int& cnt)
    {
        cnt = 0;
        insert(match_str);
        int len = strlen(src);
        for (int i = 0; i < len; i ++ )
        {
            TrieNode* node = this->root;
            for (int j = i; j < len; j ++ )
            {
                if (node->son.find(src[j]) == node->son.end()) break;
                node = node->son[src[j]];
                // 匹配成功，保存位置
                if (node->isEnd) pos[cnt ++ ] = i;
            }
        }
        return cnt > 0;
    }

private:
    TrieNode* root;
};


/* 函数的声明 */

/* 显示函数 */
void showMainMenu();
void showHelpMenu();
void showActiveArea(const ActiveArea&, int, int, int);
void printLine(const LineBlock*);
void printPositions(Pos[], int);
/* 辅助函数 */
void readFile(char*, char*);
int getValidInput();
void getNumber(const char*, int&, int, int);
void getString(const char*, char*, int, int);
bool CHECK_AREA(const ActiveArea&);
bool CHECK_BLOCK(const LineBlock*);
bool CHECK_BOUND(int, int, int, int low = 0);
/* 对Line操作的一些函数 */
bool initArea(ActiveArea&);
int countLine(const ActiveArea&);
bool emptyArea(const ActiveArea&);
bool insertLine(ActiveArea&, char*, int, ActiveArea&, char* output_file = nullptr);
void changeActiveArea(ActiveArea&, ActiveArea&, char*);
bool deleteLine(ActiveArea&, ActiveArea&, int, int);
bool matchString(ActiveArea&, char*, int, int, Pos[], int&, int);
bool replaceString(ActiveArea&, int, char*, char*, int&, int type = Hazel::KMP);
bool replaceString(ActiveArea&, char*, char*, int, int, int&, int type = Hazel::KMP);
/* 对LineBlock操作的一些函数 */
bool insertLineBlock(LineBlock*&, char*);
bool emptyLineBlock(const LineBlock*);
bool clearLineBlock(LineBlock*&);
void blocks_to_str(LineBlock*, char*);
void str_to_blocks(char*, LineBlock*&);
/* 对文件操作的一些函数 */
bool readFromInputFile(char*, ActiveArea&, ActiveArea&, char* ouput_file = nullptr);
bool writeToOutputFile(char*, ActiveArea&, ActiveArea&, int, int);
bool readFromOtherArea(ActiveArea&, ActiveArea&, int);
/* 对字符串操作的一些函数 */
bool insertString(char*, int, const char*);
bool eraseString(char*, int, int);
bool kmp(char*, char*, int[], int&);
bool trie(char*, char*, int[], int&);
bool bf(char*, char*, int[], int&);


/* 函数的具体实现 */


/* 1. 显示函数的实现 */

// 显示主操作界面
void showMainMenu()
{
    INFO("************  Welcome to Hazel Editor  ************")
    std::cout << Hazel::CYAN << "Please input your operation: (If you are new, you can type help to get help menu)" << Hazel::RESET << std::endl;
}
// 显示帮助菜单
void showHelpMenu()
{
    INFO("help menu")
    std::cout << Hazel::CYAN << "********************************************************************************" << Hazel::RESET << std::endl;
    std::cout << Hazel::CYAN << "*" << Hazel::RESET << " 1. Insert Line: Format - i<line_number><Enter><text><Enter>.\n"
        << Hazel::CYAN << "*" << Hazel::RESET << " 2. Delete Line: Format - d<line_number1>[ <line_number2>].\n"
        << Hazel::CYAN << "*" << Hazel::RESET << " 3. Switch Active Area: Format - n\n"
        << Hazel::CYAN << "*" << Hazel::RESET << " 4. Display Active Area: Format - p\n"
        << Hazel::CYAN << "*" << Hazel::RESET << " 5. Replace String: Format - S<line_number1>( <line_number2>)@<string1>@<string2>.\n"
        << Hazel::CYAN << "*" << Hazel::RESET << " 6.  Match String: Format - m <string>[ <line_number1>[ <line_number2>]].\n"
        << Hazel::CYAN << "*" << Hazel::RESET << " 7. Read From File: Format - r <file_name>.\n"
        << Hazel::CYAN << "*" << Hazel::RESET << " 8. Write To File: Format - w <file_name>[ <line_number1>[ <line_number2>]].\n"
        << Hazel::CYAN << "*" << Hazel::RESET << " 9. Exit Editor: Format - exit" << std::endl;
    std::cout << Hazel::CYAN << "********************************************************************************" << Hazel::RESET << std::endl;
}
// 显示活区，从第begin行开始显示，一次显示cnt行
void showActiveArea(const ActiveArea& activeArea, int begin, int cnt, int page_num)
{
    INFO("---------------  Current Active Area (Page " << page_num << ")  ---------------")
    if (emptyArea(activeArea))
    {
        WARNINGS("    Current Active Area is None!\n")
        return;
    }
    // 输出当前活区
    Line* cur = activeArea;
    while (cur && begin -- ) cur = cur->next;  // 找出要显示的活区起始行第begin行
    if (!cur)  // 若cur为空，说明活区遍历到尾了
    {
        WARNINGS("    Current Active Area is None!\n")
        return;
    }
    // 此时cur指向第begin行
    while (cur && cnt -- )
    {
        std::cout << Hazel::BLUE << std::setw(4) << cur->line_no << Hazel::RESET << ' ';  // 显示行号（蓝色显示）
        printLine(cur->content);
        cur = cur->next;
    }
    std::cout << Hazel::YELLOW << std::setw(60) << "total line: " << activeArea->line_no << Hazel::RESET << std::endl;  // 显示总行数（黄色显示）
    std::cout << std::endl;
}
// 打印一行的文本内容
void printLine(const LineBlock* line_content)
{
    while (line_content)
    {
        std::cout << line_content->data;
        line_content = line_content->next;
    }
    std::cout << std::endl;
}
// 打印位置(row,col)函数
void printPositions(Pos positions[], int n)
{
    for (int i = 0; i < n; i ++ )
        std::cout << i + 1 << "-th match position: " << Hazel::CYAN <<positions[i].row << "," << positions[i].col << Hazel::RESET << std::endl;
}


/* 2. 辅助函数的实现 */

// 读取输入输出文件名
void readFile(char* input_file, char* output_file)
{
    std::cout << "Please enter Input File: ";
    std::cin.getline(input_file, Hazel::MAXFILENAMELEN);
    std::cout << "Please enter Output File: ";
    std::cin.getline(output_file, Hazel::MAXFILENAMELEN);
}
// 获取有效输入
int getValidInput()
{
    int input;
    while (true)
    {
        std::cin >> input;
        if (std::cin.fail())  // 检查输入有效性
        {
            WARNINGS("Invalid Input! Please input a number")
            std::cin.clear();  // 清除错误状态
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');  // 丢弃无效的输入
        }
        else break;
    }
    return input;
}
// 读取串src中从下标为[begin,end]的区间内的数字给res （若end超出范围表示读到字符串结束）
void getNumber(const char* src, int& res, int begin, int end)
{
    if (!CHECK_BOUND(begin, end, strlen(src), 0)) return;
    res = 0;
    for (int i = begin; src[i] && i <= end; i ++ ) res = res * 10 + (src[i] - '0');
}
// 读取src的字串src[begin,end]给dist
void getString(const char* src, char* dist, int begin, int end)
{
    if (!CHECK_BOUND(begin, end, strlen(src), 0)) return;
    int cnt = 0;
    for (int i = begin; i <= end; i ++ ) dist[cnt ++ ] = src[i];
    dist[cnt] = '\0';
}
// 检查ActiveArea是否合法
bool CHECK_AREA(const ActiveArea& area)
{
    if (!area)
    {
        ERROR("The area is not exist")
        return false;
    }
    return true;
}
// 检查行块是否合法
bool CHECK_BLOCK(const LineBlock* block)
{
    if (emptyLineBlock(block))
    {
        ERROR("The block is not exist")
        return false;
    }
    return true;
}
// 检查区间[begin,end]是否合法，high为区间的上界，low为下界（默认为0）
bool CHECK_BOUND(int begin, int end, int high, int low)
{  // 也可以改成抛出异常
    if (begin < low || begin > high)
    {
        ERROR("The parameter 'begin' is invalid")
        // throw std::runtime_error("index out of bounds");
        return false;
    }
    if (end < low || end > high)
    {
        ERROR("The parameter 'end' is invalid")
        // throw std::runtime_error("index out of bounds");
        return false;
    }
    if (begin > end)
    {
        ERROR("Invalid interval")
        // throw std::runtime_error("invalid interval");
        return false;
    }
    return true;
}
// 清屏函数
void clearScreen()
{
    #ifdef _WIN32
    system("cls");
    #elif __linux__
    system("clear");
    #endif
}


/* 3. 对Area操作的相关函数的实现 */

// 初始化ActiveArea
bool initArea(ActiveArea& area)
{
    try
    {
        area = new Line(0, nullptr, nullptr);
    }
    catch (std::bad_alloc& e)
    {
        ERROR(e.what())
        return false;
    }
    return true;
}
// 返回活区的长度（总行数）
int countLine(const ActiveArea& area)
{
    if (!CHECK_AREA(area)) return -1;
    return area->line_no;
}
// 判断ActiveArea是否为空
bool emptyArea(const ActiveArea& area)
{
    if (!CHECK_AREA(area)) return false;
    return area->line_no == 0/* && area->next == nullptr*/;
}
// 将行插入到第i行后面，当output_file为空时说明正在往非活区中插入新行
bool insertLine(ActiveArea& activeArea, char* text, int i, ActiveArea& otherArea, char* output_file)
{
    if (i < 0 || i > countLine(activeArea))  // 插入位置不合法
    {
        ERROR("The position of the inserted row is invalid")
        return false;
    }
    // 1. 处理输入文本
    LineBlock* line_content = nullptr;  // 新行的头指针
    str_to_blocks(text, line_content);  // 将字符串转换成块的形式存储在line_content中
    // 2. 将文本插入活区中
    Line* new_line = new Line(i + 1, line_content);  // 新行，作为第i+1行存在与活区中
    if (emptyArea(activeArea))  // 若活区中不存在任何行，则新增该行
    {
        initArea(activeArea);  // 初始化activeArea（带头节点）
        activeArea->next = new_line;
        activeArea->line_no = 1;  // 更新当前行数为1
        return true;
    }
    Line* cur = activeArea;  // cur指向插入节点的前趋节点，初始化为头节点
    while (cur->next && i -- ) cur = cur->next;  // 寻找插入位置
    if (i > 0)
    {
        ERROR("The position of the inserted row is invalid")
        return false;
    }
    new_line->next = cur->next;
    cur->next = new_line;
    // 修改new_line后面所有行的行号
    cur = new_line->next;
    while (cur)
    {
        cur->line_no ++ ;
        cur = cur->next;
    }
    activeArea->line_no ++ ;  // 更新活区（或非活区）长度
    // 只有写入活区时才需要检查：若插入新行后活区长度超出限制，则将第一行输出（只有output_file有值时，才是插入到活区中）
    if (activeArea->line_no > Hazel::MAXACTIVELEN && output_file)
        writeToOutputFile(output_file, activeArea, otherArea, 1, 1);  // 将活区中的第1行输出
    return true;
}
// 活区切换，将活区写入输出文件，并从非活区中读取下一段，作为新的活区。
void changeActiveArea(ActiveArea& activeArea, ActiveArea& otherArea, char* output_file)
{
    if (!CHECK_AREA(activeArea)) return;
    int len = countLine(activeArea);  // len存储要把活区中多少行的内容写入输出文件，默认为全部
    // 若输入文件尚未读完，活区切换命令可将原活区中最后10行留在活区顶部，以保持阅读连续性；否则，它意味着结束编辑或开始编辑另一个文件
    if (countLine(otherArea)) len -= 10;
    writeToOutputFile(output_file, activeArea, otherArea, 1, len);
}
// 删除活区中[begin,end]区间的所有行，若非活区中还存在内容，则将非活区中的内容接到活区的最后一行后面
bool deleteLine(ActiveArea& activeArea, ActiveArea& otherArea, int begin, int end)
{
    if (!CHECK_AREA(activeArea)) return false;
    if (!CHECK_BOUND(begin, end, countLine(activeArea), 1)) return false;
    // 删除[begin,end]内的所有行
    int len = end - begin + 1;  // len存储删除的总行数
    Line* cur = activeArea;
    while (cur->next && -- begin) cur = cur->next;  // 让cur找到第begin行的前一行
    Line* left = cur;  // left指向第begin行的前一行
    // 删除[begin,end]内的所有行，并让cur指向第end行的后一行
    int tmp_length = len;  // 临时变量存储len
    cur = cur->next;  // 此时cur指向第beg行
    while (cur && tmp_length -- )  // 依次删除len行
    {
        Line* tmp = cur;
        cur = cur->next;
        delete tmp;
    } // 此时cur指向第end行的后一行
    left->next = cur;
    // 修正end行后面所有行的行号
    while (cur)
    {
        cur->line_no -= len;
        cur = cur->next;
    }
    // 更新行数
    activeArea->line_no -= len;
    // 若非活区中还有内容，则读入到活区中，读入的行数与删除的行数相同
    if (!emptyArea(otherArea)) readFromOtherArea(activeArea, otherArea, len);
    return true;
}
// 在活区activeArea中的[begin,end]行区间内匹配字符串match_str，并把结果放入Pos数组中，cnt为Pos数组长度，type指定匹配算法，默认使用KMP算法进行匹配。若匹配成功，返回true
bool matchString(ActiveArea& activeArea, char* match_str, int begin, int end, Pos positions[], int& cnt, int type = Hazel::KMP)
{
    if (!CHECK_AREA(activeArea) || !CHECK_BOUND(begin, end, countLine(activeArea), 1)) return false;
    cnt = 0;  // cnt初始化成0
    int len = end - begin + 1;  // len存储总共有多少行需要匹配
    Line* cur = activeArea;
    while (cur->next && begin -- ) cur = cur->next;  // 找到第begin行，并让cnt指向第begin行
    while (cur && len -- )
    {
        char line_content[Hazel::MAXLINELEN];  // 存储当前行的文本内容
        blocks_to_str(cur->content, line_content);  // 将当前行块转成一个大的字符数组
        // 在当前行内进行匹配（kmp）
        int pos[Hazel::MAXLINELEN] = {0}, matched_cnt = 0;  // pos中位置下标从0开始，因此下边需要+1
        // 指定匹配算法
        if (type == Hazel::KMP) kmp(line_content, match_str, pos, matched_cnt);
        else if (type == Hazel::TRIE) trie(line_content, match_str, pos, matched_cnt);
        else if (type == Hazel::BF) bf(line_content, match_str, pos, matched_cnt);
        else kmp(line_content, match_str, pos, matched_cnt);
        // 将匹配成功的字符串位置存入positions中
        for (int i = 0; i < matched_cnt; i ++ ) positions[cnt ++ ] = {cur->line_no, pos[i] + 1};
        // 继续进行下一行匹配
        cur = cur->next;
    }
    INFO("Match Ends")
    return cnt > 0;  // 若cnt大于0，说明匹配到了，否则，没有匹配到
}
// 将活区中第line_no行的字符串original替换成字符串replaced，cnt记录替换成功次数，type指定匹配算法，默认使用KMP算法
bool replaceString(ActiveArea& activeArea, int line_no, char* original, char* replaced, int& cnt, int type)
{
    if (!CHECK_AREA(activeArea)) return false;
    if (line_no < 1 || line_no > countLine(activeArea))
    {
        ERROR("The parameter 'line_no' is invalid")
        return false;
    }
    int tmp = line_no;  // 临时变量记录目标行号
    char line_content[Hazel::MAXLINELEN];  // 存储当前行所有文本内容
    Line* cur = activeArea;
    while (cur->next && tmp -- ) cur = cur->next;  // 找到第line_no行
    blocks_to_str(cur->content, line_content);  // 将第line_no行的每一块合并成一个完整的字符串赋值给line_content
    // 字符串匹配
    int matched[Hazel::MAXLINELEN] = {0};  // matched存储匹配成功的下标
    // 指定匹配算法
    if (type == Hazel::KMP) kmp(line_content, original, matched, cnt);
    else if (type == Hazel::TRIE) trie(line_content, original, matched, cnt);
    else if (type == Hazel::BF) bf(line_content, original, matched, cnt);
    else kmp(line_content, original, matched, cnt);
    // 枚举每个替换点
    int original_length = strlen(original);
    int replaced_length = strlen(replaced);
    int last = 0;  // last存储替换成新串后整个字符串的偏移量
    for (int i = 0; i < cnt; i ++ )
    {
        int idx = matched[i] + last;  // 获取匹配到的每个字符在母串line_content中的起点下标，需要加上偏移量
        // idx始终指向匹配到的字符串在line_content中的下标
        if (idx > Hazel::MAXLINELEN) break;  // 若超出范围，直接退出，替换结束
        eraseString(line_content, idx, idx + original_length - 1);  // 删除字符串line_content中的original字符串
        insertString(line_content, idx, replaced);  // 在第idx个字符后面插入字符串replaced
        last += replaced_length - original_length;  // 更新偏移量last
    }
    str_to_blocks(line_content, cur->content);  // 将修改后的line_content“还给”cur->content
    // !注意：若用临时变量LineBlock* content记录cur->content不会把操作完的结果映射到activeArea上
    return cnt > 0;
}
// 将活区中第[begin,end]区间内所有行的字符串original替换成字符串replaced，cnt记录替换成功次数，type指定匹配算法，默认使用KMP算法
bool replaceString(ActiveArea& activeArea, char* original, char* replaced, int begin, int end, int& cnt, int type)
{
    if (!CHECK_AREA(activeArea) || !CHECK_BOUND(begin, end, countLine(activeArea), 1)) return false;
    for (int i = begin; i <= end; i ++ )
    {
        int line_match_cnt = 0;  // 存储每一行成功匹配的次数
        replaceString(activeArea, i, original, replaced, line_match_cnt, type);
        cnt += line_match_cnt;
    }
    INFO("Replace Ends")
    return cnt > 0;
}

/* 4. LineBlock相关函数的实现 */

// 将行块插入到line_block后面，尾插法
bool insertLineBlock(LineBlock*& line_block, char* text)
{
    if (!line_block)  // 若该行为空，则直接插入
    {
        line_block = new LineBlock(text);
        return true;
    }
    // 插入到行的尾部。因为这只是一个行编辑器。
    LineBlock* cur = line_block;
    while (cur->next) cur = cur->next;
    LineBlock* new_block = new LineBlock(text);
    cur->next = new_block;
    return true;
}
// 判断LineBlock是否为空
bool emptyLineBlock(const LineBlock* block)
{
    return block == nullptr;
}
// 清空LineBlock
bool clearLineBlock(LineBlock*& blocks)
{
    if (!CHECK_BLOCK(blocks)) return false;
    // 清空每一块字符数组
    while (blocks)
    {
        LineBlock* tmp = blocks;  // 临时指针，指向要删除的那个节点
        blocks = blocks->next;  // 头指针后移
        delete tmp;  // 删除节点
    }
    blocks = nullptr;  // 置空
    return true;
}
// 将src中的每一块拷贝给字符数组dist
void blocks_to_str(LineBlock* src, char* dist)
{
    LineBlock* cur = src;
    // 将每一块拷贝给dist
    int len = 0;  // len存储dist字符数组长度
    while (cur)
    {
        for (int i = 0; cur->data[i]; i ++ ) dist[len ++ ] = cur->data[i];  // 按字符拷贝
        cur = cur->next;
    }
    dist[len] = '\0';  // 标记字符结束符
}
// 将字符数组src按照80个字符一块拷贝给dist
void str_to_blocks(char* src, LineBlock*& dist)
{
    if (!emptyLineBlock(dist)) clearLineBlock(dist);  // 先清空dist
    // 再将src分块
    int len = strlen(src);  // len存储当前还剩下多少字符没处理
    char block_content[Hazel::MAXBLOCKLEN];  // 存储每一块的文本
    int idx = 0;  // 记录src的下标索引
    while (len > 0)  // 依次处理每一块
    {
        int block_len = 0;  // 记录当前块的大小
        for (int i = idx; src[i]; i ++ )  // 将src中的内容拷贝到每一块中
        {
            if (block_len == Hazel::MAXBLOCKLEN - 1) break;
            block_content[block_len ++ ] = src[i];
        }
        block_content[block_len] = '\0';  // 将最后一个字符置为'\0'，表示块结束
        insertLineBlock(dist, block_content);  // 将该块内容插入到dist中
        len -= Hazel::MAXBLOCKLEN - 1;  // 更新len
    }
}


/* 5. 对文件操作的一些函数 */

// 将输入文件中的内容读入活区中，读取失败返回false，若output_file为空，则表示读入非活区中
bool readFromInputFile(char* input_file, ActiveArea& activeArea, ActiveArea& otherArea, char* output_file)
{
    std::ifstream ifs(input_file);
    if (!ifs.is_open())
    {
        ERROR("Input File: '" << input_file << "' opening failed")
        return false;
    }
    std::cout << "Reading from the file: '" << Hazel::MAGENTA << input_file << Hazel::RESET <<  "' ...\n";
    char text[Hazel::MAXLINELEN];
    int cnt = countLine(activeArea);  // cnt记录当前活区行数
    // 说明正在往活区中读入内容（此时非活区必须为空）
    if (output_file != nullptr)
    {
        while (ifs.getline(text, Hazel::MAXLINELEN) && cnt < Hazel::MAXACTIVELEN - 20)  // 最多读取到ACTIVEMAXLEN - 20行
            insertLine(activeArea, text, cnt ++ , otherArea, output_file);  // 将text插入到第cnt行后面
        activeArea->line_no = cnt;  // 更新活区的长度（在insertLine中其实会自动更新当前活区长度）
        // 将剩余部分读入非活区，非活区无长度限制
        if (strlen(text) != 0) insertLine(otherArea, text, 0, activeArea, nullptr);  // 读取第81行到非活区中
        cnt = 1;  // 此时cnt记录非活区长度，接下来从第82行开始读，并且从otherArea的第1行开始写
        while (ifs.getline(text, Hazel::MAXLINELEN)) insertLine(otherArea, text, cnt ++ , activeArea, nullptr);  // 剩余部分读入非活区
    }
    // 说明正在往非活区中读入内容
    else
    {
        while (ifs.getline(text, Hazel::MAXLINELEN)) insertLine(otherArea, text, cnt ++ , activeArea, nullptr);
    }
    INFO("End of Reading")
    ifs.close();  // 关闭文件输入流
    return true;
}
// 将活区中区间[begin, end]行的内容写入输出文件
bool writeToOutputFile(char* output_file, ActiveArea& activeArea, ActiveArea& otherArea, int begin, int end)
{
    std::ofstream ofs(output_file, std::ios::app);  // 以追加的方式打开文件
    if (!ofs.is_open())
    {
        ERROR("Output file opening failed")
        return false;
    }
    if (!CHECK_BOUND(begin, end, countLine(activeArea))) return false;
    // 将区间内的所有行写入输出文件中
    std::cout << "Writing to the file: '" << Hazel::MAGENTA << output_file << Hazel::RESET <<"' ...\n";
    int len = end - begin + 1;  // len存储要写入输出文件的总行数
    char text[Hazel::MAXBLOCKLEN];
    Line* cur = activeArea;
    while (cur->next && -- begin) cur = cur->next;  // 找到第begin行的前趋节点
    Line* left = cur;  // left指向第begin行的前趋
    cur = cur->next;  // 让cur指向第begin行
    for (int i = 0; i < len && cur; i ++ )  // 依次写入第begin行到第end行
    {
        Line* tmp = cur;
        char text[Hazel::MAXLINELEN];
        blocks_to_str(cur->content, text);  // 将行块转成char字符数组
        ofs << text << std::endl;  // 依次写入输出文件中
        cur = cur->next;  // 继续下一行
        clearLineBlock(tmp->content);  // 清空当前行
        delete tmp;
    }
    left->next = cur;  // 将第left行和end后一行连接起来
    // 修正第left行后面所有行的行号
    while (cur)
    {
        cur->line_no -= end;  // 后面的行号减去end即可
        cur = cur->next;
    }
    activeArea->line_no -= len;  // 更新活区当前总行数
    // 若非活区中还有内容，则从非活区中读入len行到活区中
    if (!emptyArea(otherArea))
    {
        readFromOtherArea(activeArea, otherArea, len);  // 需要读取len行非活区的内容
    }
    INFO("End of Writing")
    ofs.close();  // 关闭输出文件流
    return true;
}
// 将非活区中的len行写入活区中 （若不足len行，则全部写入活区）
bool readFromOtherArea(ActiveArea& activeArea, ActiveArea& otherArea, int len)
{
    if (!CHECK_AREA(activeArea) || !CHECK_AREA(otherArea)) return false;
    int actualLength = 0;  // 存储实际写入的行数
    Line* tail = activeArea;  // tail指向活区的尾节点
    while (tail->next) tail = tail->next;
    // 将otherArea的前len行读入activeArea
    Line* head = otherArea->next;  // head指向非活区的首元节点
    while (head && len -- )
    {
        tail->next = head;
        otherArea->next = head->next;  // otherArea的首元节点后移
        head = otherArea->next;  // 更新head始终指向非活区的首元节点
        tail->next->line_no += 10;  // 非活区读到活区的每一行的line_no都要加上偏移量(10)
        tail = tail->next;  // tail后移
        tail->next = nullptr;  // tail的next指针置空
        actualLength ++ ;  // 实际写入行数+1
    }
    otherArea->next = head;  // 更新otherArea的next指针指向非活区的首元节点head
    // 更新活区和非活区的长度
    otherArea->line_no -= actualLength;
    activeArea->line_no += actualLength;
    INFO("Read from Other Area successfully")
    return true;
}


/* 6. 字符串相关函数的实现 */

// 在str中第pos个字符的后面插入字符串value
bool insertString(char* str, int pos, const char* value)
{
    int n = strlen(str);
    if (pos < 0 || pos > n)  // 插入位置不合法
    {
        ERROR("The parameter 'pos' is invalid")
        return false;
    }
    int len = strlen(value);  // len表示要插入的字符串的长度
    // 先将原串插入位置后面的所有字符向后偏移len位，为value的插入腾出空间来
    for (int i = n + len; i > pos + len - 1; i -- )
    {
        if (i >= Hazel::MAXLINELEN - 1) continue;  // 如果超出范围，则丢弃
        str[i] = str[i - len];
    }
    // 再把要添加的字符串value拷贝到腾出的那块空间中
    for (int i = pos; i < pos + len; i ++ ) str[i] = value[i - pos];
    // 最后标记上字符结束符即可
    str[n + len] = '\0';
    return true;
}
// 删除字符串str中[begin,end]区间内的字符
bool eraseString(char* str, int begin, int end)
{
    int n = strlen(str);
    if (!CHECK_BOUND(begin, end, n)) return false;
    int len = end - begin + 1;  // 存储要删除的字符长度
    for (int i = end + 1; str[i]; i ++ ) str[i - len] = str[i];  // 将后面的字符往前移len个单位
    str[n - len] = '\0';  // 标记字符结束符
    return true;
}
// kmp，O(n)，匹配母串str中的所有模式串match_str，并将其位置放入pos数组中，长度为cnt。若匹配成功，返回true
bool kmp(char* str, char* match_str, int pos[], int& cnt)
{
    cnt = 0;  // 初始化cnt为0
    int n = strlen(match_str), m = strlen(str);  // n为模式串长度，m为母串长度
    if (n > m) return false;
    char p[Hazel::MAXLINELEN + 1], s[Hazel::MAXLINELEN + 1];  // p：模式串，s：母串。偏移量+1，更好计算
    memcpy(p + 1, match_str, std::min(n, Hazel::MAXLINELEN - 1));
    memcpy(s + 1, str, std::min(m, Hazel::MAXLINELEN - 1));
    p[n + 1] = s[m + 1] = '\0';  // 标记结束符
    // 求next数组
    int ne[Hazel::MAXLINELEN + 1] = {0};
    for (int i = 2, j = 0; i <= n; i ++ )
    { // ne[1] = 0，因为如果第一个匹配不上，就只能从0开始重新匹配了
        while (j && p[i] != p[j + 1]) j = ne[j]; // 与KMP匹配过程类似
        if (p[i] == p[j + 1]) j ++ ;
        ne[i] = j; // 记录ne[i]的值为j
    }
    // KMP匹配过程
    for (int i = 1, j = 0; i <= m; i ++ )
    {
        while (j && s[i] != p[j + 1]) j = ne[j]; // 如果s[i]与p[j + 1]不能匹配，将j移动到ne[j]的位置继续与s[i]匹配，直到j为0
        if (s[i] == p[j + 1]) j ++ ; // 匹配成功，j向后移动
        if (j == n) // 此时，整个字符串都匹配成功
        {
            pos[cnt ++ ] = i - n;  // 将匹配成功的位置存储pos数组中，下标从0开始
            j = ne[j]; // 从ne[j]开始，继续下一次匹配
        }
    }
    return cnt > 0;
}
// trie，O(n)，匹配母串str中的所有模式串match_str，并将其位置放入pos数组中，长度为cnt。若匹配成功，返回true
bool trie(char* src, char* match_str, int pos[], int& cnt)
{
    Trie tr;
    if (strlen(match_str) > strlen(src)) return false;
    return tr.search(src, match_str, pos, cnt);
}
// bf，O(nm)，匹配母串str中的所有模式串match_str，并将其位置放入pos数组中，长度为cnt。若匹配成功，返回true
bool bf(char* str, char* match_str, int pos[], int& cnt)
{
    cnt = 0;
    int n = strlen(str), m = strlen(match_str);
    if (m > n) return false;
    // 暴力枚举每次匹配的起点
    for (int i = 0; i <= n - m; i ++ )
    {
        int j;
        for (j = 0; j < m; j ++ )  // 枚举匹配长度
            if (str[i + j] != match_str[j])
                break;
        // 若匹配成功，保存位置
        if (j == m) pos[cnt ++ ] = i;
    }
    return cnt > 0;
}


/* 主函数 */
int main()
{
    ActiveArea activeArea = nullptr, otherArea = nullptr;
    initArea(activeArea);
    initArea(otherArea);

    char input_file[Hazel::MAXFILENAMELEN + 1] = "../data/input/input.txt", output_file[Hazel::MAXFILENAMELEN + 1] = "../data/output/output.txt";
    readFile(input_file, output_file);
    
    // 若有输入文件，则先让程序从输入文件中读取文本
    if (input_file)
    {
        readFromInputFile(input_file, activeArea, otherArea, output_file);
    }
    // 开始读入操作
    char op[Hazel::MAXLINELEN] = {0};
    int count = 0;  // count记录当前已经进行多少次操作
    while (true)
    {
        // 打印上次操作
        TRACE("last operation: " + (strlen(op) == 0 ? std::string("none") : std::string(op)))
        // 显示活区
        showActiveArea(activeArea, 1, countLine(activeArea), 1);
        // 展示主操作界面
        showMainMenu();
        // 读入本次操作
        std::cin.getline(op, sizeof op);
        // 操作次数+1
        count ++ ;

        /* 判断操作类型 */
        // 1. 退出系统
        if (strcmp(op, "exit") == 0)
        {
            break;
        }
        // 2. 显示帮助菜单
        else if (strcmp(op, "help") == 0)
        {
            showHelpMenu();
            continue;
        }
        // 3. 行插入
        else if (op[0] == 'i')
        {
            char str[Hazel::MAXLINELEN];
            std::cin.getline(str, sizeof str);
            int idx = 0;
            getNumber(op, idx, 1, strlen(op) - 1);  // 插入到第idx行
            insertLine(activeArea, str, idx, otherArea, output_file);  // 调用插入函数
        }
        // 4. 删除行
        else if (op[0] == 'd')
        {
            int i;
            for (i = 1; op[i] && op[i] != ' '; i ++ );  // 让i指向空格位置
            int begin = 0, end = 0;  // 删除行的上下界
            getNumber(op, begin, 1, i - 1);
            if (op[i] && op[i] == ' ') getNumber(op, end, i + 1, strlen(op) - 1);
            else end = begin;
            // 若可删除，则调用删除函数
            if (!emptyArea(activeArea)) deleteLine(activeArea, otherArea, begin, end);
        }
        // 5. 活区切换
        else if (op[0] == 'n' && strlen(op) == 1)
        {
            changeActiveArea(activeArea, otherArea, output_file);  // 调用活区切换函数
        }
        // 6. 活区显示
        else if (op[0] == 'p' && strlen(op) == 1/* || count == 3*/)
        {
            int cur_line = 1, show_lines = 20;  // 起始显示行数和一次展示多少行
            int page_num = 1;  // 存储当前页数
            while (true)
            {
                showActiveArea(activeArea, cur_line, show_lines, page_num ++ );  // 显示一次后页数+1
                cur_line += show_lines;
                if (cur_line > countLine(activeArea))  // 显示完毕，退出
                {
                    TRACE("End of Active Area")
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');  // 过滤最后一个'Y'的回车
                    break;
                }
                std::cout << "Do you want to continue to the next page? (Y/N): ";
                char choice; std::cin >> choice;  // 输入选择
                if (choice != 'Y') break;  // 若不继续显示或者显示完毕，退出
            }
        }
        // 7. 字符串替换
        else if (op[0] == 'S')
        {
            int begin = 0, end = 0;
            int i, j;
            for (i = 1; op[i] != '@'; i ++ );  // 让i指向第一个'@'处
            for (j = i + 1; op[j] != '@'; j ++ );  // 让j指向第二个'@'处
            char original[Hazel::MAXLINELEN], replaced[Hazel::MAXLINELEN];
            int k = 1;
            while (op[k] && op[k] != ',') k ++ ;  // 让k指向逗号处（或字符串结束符'\0'处）
            if (op[k] == ',')  // 说明存在<行号2>
            {
                getNumber(op, begin, 1, k - 1);  // 获取<行号1>
                getNumber(op, end, k + 1, i - 1);  // 获取<行号2>
            }
            else
            {
                getNumber(op, begin, 1, i - 1);  // 仅获取<行号1>
                end = begin;
            }
            getString(op, original, i + 1, j - 1);  // 获取原串
            getString(op, replaced, j + 1, strlen(op) - 1); // 获取替换串
            int replaced_cnt = 0;  // 存储替换次数
            replaceString(activeArea, original, replaced, begin, end, replaced_cnt);  // 调用替换字符串函数（默认使用KMP）
            std::cout << Hazel::CYAN << "total replaced counts: " << replaced_cnt << std::endl;
        }
        // 8. 字符串匹配
        else if (op[0] == 'm')
        {
            char match_str[Hazel::MAXLINELEN];
            int i = 1;
            while (op[i] && op[i] == ' ') i ++ ;  // 过滤空格，让i指向待匹配串的起始下标
            int j = i;
            while (op[j] && op[j] != ' ') j ++ ;  // 让j指向下一个空格处（或字符串结束符'\0'处）
            getString(op, match_str, i, j - 1);  // 获取待匹配字符串
            i = j;
            while (op[i] && op[i] == ' ') i ++ ;  // 过滤空格，让i指向<行号1>起始下标处
            j = i;
            while (op[j] && op[j] != ' ') j ++ ;  // 过滤<行号1>，让j指向下一个空格处（或'\0'处）
            int begin = 0, end = 0;
            if (i == j)  // 说明只输入的匹配串
            {
                begin = 1, end = countLine(activeArea);
            }
            else
            {
                getNumber(op, begin, i, j - 1);  // 获取行号1
                i = j;
                while (op[i] && op[i] == ' ') i ++ ;  // 过滤空格，让i指向<行号2>起始下标处
                j = i;
                while (op[j] && op[j] != ' ') j ++ ;  // 过滤<行号2>，让j指向最终结束符'\0'处
                if (i == j)  // 说明只输入了<行号1>
                    end = begin;
                else  // 否则，获取<行号2>
                    getNumber(op, end, i, j - 1);
            }
            Pos positions[Hazel::MAXLINELEN * Hazel::MAXACTIVELEN] = {0};  // 存储匹配成功的位置
            int matched_cnt = 0;  // 记录匹配成功的个数
            // 匹配活区内所有match
            bool is_matched = matchString(activeArea, match_str, begin, end, positions, matched_cnt);
            std::cout << Hazel::CYAN << "total matches: " << matched_cnt << Hazel::RESET << std::endl;
            if (is_matched) printPositions(positions, matched_cnt);  // 若匹配成功，则打印所有匹配到的位置
        }
        // 9. 读取输入文件到活区中（前提是非活区中无内容）
        else if (op[0] == 'r')
        {
            int i = 1;
            while (op[i] && op[i] == ' ') i ++ ;  // 过滤空格，i指向<文件名>起始下标
            getString(op, input_file, i, strlen(op) - 1);
            if (!emptyArea(otherArea))  // 若非活区中还存在内容，则无法读取
            {
                WARNINGS("Other Area is not empty, can't read from Input File: '" <<  input_file << "\'")
                continue;
            }
            readFromInputFile(input_file, activeArea, otherArea, output_file);
        }
        // 10. 写入输出文件
        else if (op[0] == 'w')
        {
            int i = 1;
            while (op[i] && op[i] == ' ') i ++ ;  // 过滤空格，i指向<文件名>起始下标
            int j = i;
            while (op[j] && op[j] != ' ') j ++ ;  // 过滤文件名，j指向下一个空格处（或'\0'处）
            getString(op, output_file, i, j - 1);  // 获取<文件名>
            while (op[j] && op[j] == ' ') j ++ ;  // 过滤空格，j指向<行号1>处
            i = j;
            while (op[i] && op[i] != ' ') i ++ ;  // 过滤第一个数字，i指向下一个空格处（或'\0'处）
            int begin = 0, end = 0;
            if (i == j) begin = 1, end = countLine(activeArea);  // 说明只输入了<文件名>
            else
            {
                getNumber(op, begin, j, i - 1);  // 获取<行号1>
                while (op[i] && op[i] == ' ') i ++ ;  // 过滤空格，i指向<行号2>处
                j = i;
                while (op[j]) j ++ ;  // 过滤第二个数字，j指向最后字符串结束符'\0'处
                if (i == j) end = begin;  // 说明只输入了<行号1>
                else getNumber(op, end, i, j - 1);  // 获取<行号2>
            }
            // 写入获取中区间[begin,end]行的内容到输出文件中
            writeToOutputFile(output_file, activeArea, otherArea, begin, end);
        }
        // 9. 非法输入
        else
        {
            if (count == 1)  // 第一次不是非法操作
            {
                clearScreen();
                continue;
            }
            WARNINGS("Invalid Input!")
        }
        // 按下回车以继续
        std::cout << Hazel::GREEN << "Press enter to continue..." << Hazel::RESET;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');  // 判断只接收回车
        clearScreen();  // 每操作一次后都清屏
    }

    return 0;
}