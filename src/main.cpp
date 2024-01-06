#include <iostream>
#include <algorithm>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <unordered_map>

using namespace std;

#define ERROR(content)\
        cerr << Hazel::RED << "Error: " << Hazel::YELLOW << __FILE__ << Hazel::RESET\
            << " : in function " << Hazel::CYAN <<  __func__ << Hazel::RESET\
            << " at line " << Hazel::MAGENTA << __LINE__ << Hazel::RESET << endl\
            << "        Compiled on " << __DATE__\
            << " at " << __TIME__ << endl\
            << "        " << Hazel::RED << content << Hazel::RESET << endl;

const int ACTIVEMAXLEN = 100;  // 活区的大小
const int MAXLINESIZE = 320;  // 每一行的最大长度
const int MAXBLOCKSIZE = 81;  // 每一个行块的最大长度


// 定义一些颜色变量，用于输出
namespace Hazel
{
    const char* RESET = "\033[0m";
    const char* BLACK = "\033[30m";
    const char* RED = "\033[31m";
    const char* GREEN = "\033[32m";
    const char* YELLOW = "\033[33m";
    const char* BLUE = "\033[34m";
    const char* MAGENTA = "\033[35m";
    const char* CYAN = "\033[36m";
    const int KMP = 1;
    const int TRIE = 2;
    const int BF = 3;
};

// 存储活区中匹配到的字符串所处的位置
typedef struct Position {
    int row, col;
} Pos;
// 存储每一行的文本，链式存储
typedef struct LineBlock {
    char data[MAXBLOCKSIZE];  // 行中每一块的字符内容
    LineBlock* next;  // 指向下一个行块
    // 默认构造
    LineBlock(const char* text, LineBlock* ne = nullptr)
    {
        int length = strlen(text);
        memcpy(data, text, min(length, MAXLINESIZE - 1));  // 把text的内容拷贝给data
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

// Trie 节点
struct TrieNode {
    bool isEnd;  // 标记是否是单词的结尾
    std::unordered_map<char, TrieNode*> son;  // 子节点集合
    TrieNode() : isEnd(false) {}
};
// Trie 树
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

// 函数的声明
bool initLine(Line*&);
bool readFromInputFile(char*, char*, ActiveArea&, ActiveArea&);
bool writeOutputFile(char*, ActiveArea&, ActiveArea&, int, int);
bool readFromOtherArea(ActiveArea&, ActiveArea&, int);
bool insertLineBlock(LineBlock*&, char*);
bool insertLine(ActiveArea&, char*, int, ActiveArea& otherArea, char* output_file = nullptr);
int countLine(const ActiveArea&);
int getValidInput();
void showMainMenu();
void showActiveArea(const ActiveArea&, int, int, int);
void changeActiveArea(ActiveArea&, ActiveArea&, char*);
bool deleteLine(ActiveArea&, ActiveArea&, int, int);
void get_number(const char*, int&, int, int);
void get_str(const char*, char*, int, int);
bool matchString(ActiveArea&, char*, int, int, Pos[], int&, int);
bool emptyArea(const ActiveArea&);
bool CHECK_AREA(const ActiveArea&);
bool CHECK_BLOCK(const LineBlock*);
bool CHECK_BOUND(int, int, int, int low = 0);
bool eraseString(char*, int, int);
bool insertString(char*, int, const char*);
bool replaceString(ActiveArea&, int, char*, char*, int&, int);
void blocks_to_str(LineBlock*, char*);
void str_to_blocks(char*, LineBlock*&);
bool clearLineBlock(LineBlock*&);
void printPositions(Pos[], int);
void printLine(const LineBlock*);
void showHelpMenu();
bool kmp(char*, char*, int[], int&);
bool trie(char*, char*, int[], int&);
bool bf(char*, char*, int[], int&);

// 将输入文件中的内容读入活区中，读取失败返回false
bool readFromInputFile(char* input_file, char* output_file, ActiveArea& activeArea, ActiveArea& otherArea)
{
    std::ifstream ifs(input_file);
    if (!ifs.is_open())
    {
        ERROR("Input file opening failed")
        return false;
    }
    cout << "Reading from the file: '" << Hazel::MAGENTA << input_file << Hazel::RESET <<  "' ...\n";
    char text[MAXLINESIZE];
    int cnt = 0;  // 行数
    while (ifs.getline(text, MAXLINESIZE) && cnt < ACTIVEMAXLEN - 20)  // 最多读取ACTIVEMAXLEN - 20行
        insertLine(activeArea, text, cnt ++ , otherArea, output_file);  // 将text插入到第cnt行后面
    activeArea->line_no = cnt;  // 更新活区的长度
    // 将剩余部分读入非活区，非活区无长度限制
    cnt = 0;
    while (ifs.getline(text, MAXLINESIZE)) insertLine(otherArea, text, cnt, activeArea, nullptr);  // 剩余部分读入非活区
    ifs.close();  // 关闭文件输入流
    return true;
}

// 将活区中区间[begin, end]行的内容写入输出文件
bool writeOutputFile(char* output_file, ActiveArea& activeArea, ActiveArea& otherArea, int begin, int end)
{
    std::ofstream ofs(output_file, ios::app);  // 以追加的方式打开文件
    if (!ofs.is_open())
    {
        ERROR("Output file opening failed")
        return false;
    }
    if (!CHECK_BOUND(begin, end, countLine(activeArea))) return false;

    // 将区间内的所有行写入输出文件中
    cout << "Writing to the file: '" << Hazel::MAGENTA << output_file << Hazel::RESET <<"' ...\n";
    int len = end - begin + 1;  // len存储要写入输出文件的总行数
    char text[MAXBLOCKSIZE];
    Line* cur = activeArea;
    while (cur->next && -- begin) cur = cur->next;  // 找到第begin行的前趋节点
    Line* left = cur;  // left指向第begin行的前趋
    cur = cur->next;  // 让cur指向第begin行
    for (int i = 0; i < len; i ++ )  // 依次写入第begin行到第end行
    {
        Line* tmp = cur;
        char text[MAXLINESIZE];
        blocks_to_str(cur->content, text);
        ofs << text << endl;
        cur = cur->next;  // 继续下一行
        delete tmp;
    }
    left->next = cur;  // 将第left行和end后一行连接起来
    // 若非活区中还有内容，则从非活区中读入len行到活区中
    if (!emptyArea(otherArea)) readFromOtherArea(activeArea, otherArea, len);
    ofs.close();  // 关闭输出文件流
    return true;
}

// 将非活区中的len行写入活区中 （若不足len行，则全部写入活区）
bool readFromOtherArea(ActiveArea& activeArea, ActiveArea& otherArea, int len)
{
    if (!CHECK_AREA(activeArea) || !CHECK_AREA(otherArea)) return false;
    int actualLength = 0;  // 存储实际写入的行数
    Line* tail = activeArea->next;  // tail指向活区的尾节点
    while (tail) tail = tail->next;
    // 将otherArea的前len行读入activeArea
    Line* head = otherArea->next;  // head指向非活区的首元节点
    while (head && len -- )
    {
        tail->next = head;
        otherArea->next = head->next;
        head = otherArea->next;  // 更新head始终指向非活区的首元节点
        tail = tail->next;  // tail后移
        tail->next = nullptr;  // tail的next指针置空
        actualLength ++ ;  // 实际写入行数+1
    }
    // 更新活区和非活区的长度
    otherArea->line_no -= actualLength;
    activeArea->line_no += actualLength;
    return true;
}

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

// 将行插入到第i行后面，当output_file不为空时说明正在往活区中插入新行
bool insertLine(ActiveArea& activeArea, char* text, int i, ActiveArea& otherArea, char* output_file)
{
    if (i < 0 || i + 1 > ACTIVEMAXLEN)  // 插入位置不合法
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
        initLine(activeArea);
        activeArea->next = new_line;
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
    if (activeArea->line_no > ACTIVEMAXLEN && output_file)
        writeOutputFile(output_file, activeArea, otherArea, 1, 1);  // 将活区中的第1行输出
    return true;
}

// 返回活区的长度（总行数）
int countLine(const ActiveArea& area)
{
    if (emptyArea(area)) return 0;
    return area->line_no;
}

// 获取有效输入
int getValidInput()
{
    int input;
    while (true)
    {
        cin >> input;
        if (cin.fail())  // 检查输入有效性
        {
            cout << Hazel::RED << "Invalid Input! Please input a number" << Hazel::RESET << endl;
            cin.clear();  // 清除错误状态
            cin.ignore(numeric_limits<streamsize>::max(), '\n');  // 丢弃无效的输入
        }
        else break;
    }
    return input;
}

// 显示主操作界面
void showMainMenu()
{
    cout << "*****  欢迎来到行文本编辑器~  *****" << endl;
        // << "您可以选择如下操作：" << endl
        // << "1. 行插入。格式： i<行号><回车><文本><回车> ，将<文本>插入活区中第<行号>行之后；" << endl
        // << "2. 行删除。格式： d<行号1>[ <行号2>]<回车> ，删除活区中第<行号1>行（到第<行号2>行）；" << endl
        // << "3. 活区切换。格式： n<回车>" << endl
        // << "4. 活区显示。格式： p<回车>" << endl;
}

void printLine(const LineBlock* line_content)
{
    while (line_content)
    {
        cout << line_content->data;
        line_content = line_content->next;
    }
    cout << endl;
}

// 显示活区，从第begin行开始显示，一次显示cnt行
void showActiveArea(const ActiveArea& activeArea, int begin, int cnt, int page_num)
{
    cout << Hazel::GREEN << "---------------  Current Active Area (Page " << page_num << ")  ---------------" << Hazel::RESET << endl;
    if (emptyArea(activeArea))
    {
        cout << Hazel::YELLOW << setw(4) << "Current Active Area is None!\n" << Hazel::RESET << endl;
        return;
    }
    // 输出当前活区
    Line* cur = activeArea;
    while (cur && begin -- ) cur = cur->next;  // 找出要显示的活区起始行第begin行
    if (!cur)  // 若cur为空，说明活区遍历到尾了
    {
        cout << Hazel::YELLOW << setw(4) << "Current Active Area is None!\n" << Hazel::RESET << endl;
        return;
    }
    // 此时cur指向第begin行
    while (cur && cnt -- )
    {
        cout << Hazel::BLUE << std::setw(4) << cur->line_no << Hazel::RESET << ' ';  // 显示行号（蓝色显示）
        printLine(cur->content);
        cur = cur->next;
    }
    cout << Hazel::YELLOW << setw(60) << "total line: " << activeArea->line_no << Hazel::RESET << endl;
    cout << endl;
}

// 活区切换，将活区写入输出文件，并从非活区中读取下一段，作为新的活区。
void changeActiveArea(ActiveArea& activeArea, ActiveArea& otherArea, char* output_file)
{
    if (!CHECK_AREA(activeArea)) return;
    int len = countLine(activeArea);  // len存储要把活区中多少行的内容写入输出文件，默认为全部
    // 若输入文件尚未读完，活区切换命令可将原活区中最后10行留在活区顶部，以保持阅读连续性；否则，它意味着结束编辑或开始编辑另一个文件
    if (countLine(otherArea)) len -= 10;
    writeOutputFile(output_file, activeArea, otherArea, 1, len);
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

// 读取串src中从下标为[begin,end]的区间内的数字给res （若end超出范围表示读到字符串结束）
void get_number(const char* src, int& res, int begin, int end)
{
    res = 0;
    for (int i = begin; src[i] && i <= end; i ++ ) res = res * 10 + (src[i] - '0');
}

// 读取src的字串src[begin,end]给dist
void get_str(const char* src, char* dist, int begin, int end)
{
    int cnt = 0;
    for (int i = begin; i <= end; i ++ ) dist[cnt ++ ] = src[i];
    dist[cnt] = '\0';
}


// kmp，匹配母串str中的所有模式串match_str，并将其位置放入pos数组中，长度为cnt。若匹配成功，返回true
bool kmp(char* str, char* match_str, int pos[], int& cnt)
{
    cnt = 0;  // 初始化cnt为0
    int n = strlen(match_str), m = strlen(str);  // n为模式串长度，m为母串长度
    char p[MAXLINESIZE + 1], s[MAXLINESIZE + 1];  // p：模式串，s：母串。偏移量+1，更好计算
    memcpy(p + 1, match_str, min(n, MAXLINESIZE - 1));
    memcpy(s + 1, str, min(m, MAXLINESIZE - 1));
    p[n + 1] = s[m + 1] = '\0';  // 标记结束符
    // 求next数组
    int ne[MAXLINESIZE + 1] = {0};
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

// trie，匹配母串str中的所有模式串match_str，并将其位置放入pos数组中，长度为cnt。若匹配成功，返回true
bool trie(char* src, char* match_str, int pos[], int& cnt)
{
    Trie tr;
    return tr.search(src, match_str, pos, cnt);
}

// TODO:暴力匹配算法
bool bf(char* str, char* match_str, int pos[], int& cnt)
{
    cnt = 0;
    int str_len = strlen(str), match_len = strlen(match_str);
    for (int i = 0; i <= str_len - match_len; i ++ ) {
        int j;
        for (j = 0; j < match_len; j ++ )
            if (str[i + j] != match_str[j])
                break;
        // 匹配成功，保存位置
        if (j == match_len) pos[cnt ++ ] = i;
    }
    return cnt > 0;
}

// 在活区activeArea中的[begin,end]行区间内匹配字符串match_str，并把结果放入Pos数组中，cnt为Pos数组长度，type指定匹配算法，默认使用KMP算法进行匹配。若匹配成功，返回true
bool matchString(ActiveArea& activeArea, char* match_str, int begin, int end, Pos positions[], int& cnt, int type = Hazel::KMP)
{
    if (!CHECK_AREA(activeArea)) return false;
    if (!CHECK_BOUND(begin, end, countLine(activeArea))) return false;
    cnt = 0;  // cnt初始化成0
    Line* cur = activeArea;
    while (cur->next && begin -- ) cur = cur->next;  // 找到第begin行，并让cnt指向第begin行
    int len = end - begin + 1;  // len存储总共有多少行需要匹配
    while (cur->next && len -- )
    {
        char line_content[MAXLINESIZE];  // 存储当前行的文本内容
        blocks_to_str(cur->content, line_content);  // 将当前行块转成一个大的字符数组
        // 在当前行内进行匹配（kmp）
        int pos[MAXLINESIZE] = {0}, matched_cnt = 0;  // pos中位置下标从0开始，因此下边需要+1
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
    return cnt > 0;  // 若cnt大于0，说明匹配到了，否则，没有匹配到
}

// 初始化Line*(ActiveArea)
bool initLine(Line*& line)
{
    try
    {
        line = new Line(0, nullptr, nullptr);
    }
    catch (std::bad_alloc& e)
    {
        ERROR(e.what())
        return false;
    }
    return true;
}

// 判断ActiveArea是否为空
bool emptyArea(const ActiveArea& area)
{
    return area == nullptr || area->next == nullptr;
}

// 判断LineBlock是否为空
bool emptyLineBlock(const LineBlock* block)
{
    return block == nullptr;
}

// 检查ActiveArea是否合法
bool CHECK_AREA(const ActiveArea& area)
{
    if (emptyArea(area))
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
        if (i >= MAXLINESIZE - 1) continue;  // 如果超出范围，则丢弃
        str[i] = str[i - len];
    }
    // 再把要添加的字符串value拷贝到腾出的那块空间中
    for (int i = pos; i < pos + len; i ++ ) str[i] = value[i - pos];
    // 最后标记上字符结束符即可
    str[n + len] = '\0';
    return true;
}

// 将活区中第line_no行的字符串original替换成字符串replaced，type指定匹配算法，默认使用KMP算法
bool replaceString(ActiveArea& activeArea, int line_no, char* original, char* replaced, int& cnt, int type = Hazel::KMP)
{
    if (!CHECK_AREA(activeArea)) return false;
    cnt = 0;  // 初始化总替换次数cnt为0
    int tmp = line_no;  // 临时变量记录目标行号
    char line_content[MAXLINESIZE];  // 存储当前行所有文本内容
    Line* cur = activeArea;
    while (cur->next && tmp -- ) cur = cur->next;  // 找到第line_no行
    blocks_to_str(cur->content, line_content);  // 将第line_no行的每一块合并成一个完整的字符串赋值给line_content
    // 字符串匹配
    int matched[MAXLINESIZE] = {0};  // matched存储匹配成功的下标
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
        if (idx > MAXLINESIZE) break;  // 若超出范围，直接退出，替换结束
        eraseString(line_content, idx, idx + original_length - 1);  // 删除字符串line_content中的original字符串
        insertString(line_content, idx, replaced);  // 在第idx个字符后面插入字符串replaced
        last += replaced_length - original_length;  // 更新偏移量last
    }
    str_to_blocks(line_content, cur->content);  // 将修改后的line_content“还给”cur->content
    // !注意：若用临时变量LineBlock* content记录cur->content不会把操作完的结果映射到activeArea上
    return cnt > 0;
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
    char block_content[MAXBLOCKSIZE];  // 存储每一块的文本
    int idx = 0;  // 记录src的下标索引
    while (len > 0)  // 依次处理每一块
    {
        int block_len = 0;  // 记录当前块的大小
        for (int i = idx; src[i]; i ++ )  // 将src中的内容拷贝到每一块中
        {
            if (block_len == MAXBLOCKSIZE - 1) break;
            block_content[block_len ++ ] = src[i];
        }
        block_content[block_len] = '\0';  // 将最后一个字符置为'\0'，表示块结束
        insertLineBlock(dist, block_content);  // 将该块内容插入到dist中
        len -= MAXBLOCKSIZE - 1;  // 更新len
    }
    // cout << "dist->data: " << dist->data << endl;
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

void printPositions(Pos positions[], int n)
{
    for (int i = 0; i < n; i ++ )
        cout << i + 1 << "-th match position: " << Hazel::CYAN <<positions[i].row << ", " << positions[i].col << Hazel::RESET << endl;
}

void readInputFile(char* input_file, char* output_file)
{
    cout << "Please enter Input File: ";
    cin >> input_file;
    cout << "Please enter Output File: ";
    cin >> output_file;
}

void showHelpMenu()
{
    cout << Hazel::GREEN << "help menu:" << Hazel::RESET << endl;
    cout << "1. 行插入。格式： i<行号><回车><文本><回车> ，将<文本>插入活区中第<行号>行之后\n\
2. 行删除。格式： d<行号1>[ <行号2>]<回车> ，删除活区中第<行号1>行（到第<行号2>行）\n\
3. 活区切换。格式： n<回车>\n\
4. 活区显示。格式： p<回车>\n\
5. 串替换。格式： S<行号>@<串1>@<串2><回车>，将活区中第<行号>行中的<串1>替换成<串2>\n\
6. 串匹配。格式： m<串><回车>，匹配活区中所有<串>\n\
7. 退出。格式： exit\n\
";
}

int main()
{
    ActiveArea activeArea = nullptr, otherArea = nullptr;

    char input_file[100], output_file[100];
    readInputFile(input_file, output_file);
    
    // 若有输入文件，则先让程序从输入文件中读取文本
    if (input_file)
    {
        readFromInputFile(input_file, output_file, activeArea, otherArea);
    }
    // 开始读入操作
    char op[MAXLINESIZE] = {0};
    int count = 0;  // count记录当前已经进行多少次操作
    while (true)
    {
        // 打印上次操作
        cout << Hazel::MAGENTA <<"last operation: " << (strlen(op) == 0 ? "none" : op) << Hazel::RESET << endl;
        // 显示活区
        showActiveArea(activeArea, 1, countLine(activeArea), 1);
        // 展示主操作界面
        showMainMenu();
        // 读入本次操作
        cin.getline(op, sizeof op);
        // 操作次数+1
        count ++ ;

        // 判断操作类型
        // 1. 退出系统
        if (strcmp(op, "exit") == 0)
        {
            break;
        }
        // 2. 显示帮助菜单
        else if (strcmp(op, "help") == 0)
        {
            showHelpMenu();
        }
        // 3. 行插入
        else if (op[0] == 'i')
        {
            char str[MAXLINESIZE];
            cin.getline(str, sizeof str);
            int idx = 0;
            get_number(op, idx, 1, strlen(op) - 1);  // 插入到第idx行
            insertLine(activeArea, str, idx, otherArea, output_file);  // 调用插入函数
        }
        // 4. 删除行
        else if (op[0] == 'd')
        {
            int i;
            for (i = 1; op[i] && op[i] != ' '; i ++ );  // 让i指向空格位置
            int begin = 0, end = 0;  // 删除行的上下界
            get_number(op, begin, 1, i - 1);
            if (op[i] && op[i] == ' ') get_number(op, end, i + 1, strlen(op) - 1);
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
                    cout << Hazel::YELLOW << "End of Active Area" << Hazel::RESET << endl;
                    break;
                }
                cout << "Do you want to continue to the next page? (Y/N): ";
                char choice; cin >> choice;  // 输入选择
                if (choice != 'Y') break;  // 若不继续显示或者显示完毕，退出
            }
        }
        // 7. 字符串替换
        else if (op[0] == 'S')
        {
            int i, j;
            for (i = 1; op[i] != '@'; i ++ );  // 让i指向第一个'@'处
            for (j = i + 1; op[j] != '@'; j ++ );  // 让j指向第二个'@'处
            int line_no = 0;
            char original[MAXLINESIZE], replaced[MAXLINESIZE];
            get_number(op, line_no, 1, i - 1);  // 获取行号
            get_str(op, original, i + 1, j - 1);  // 获取原串
            get_str(op, replaced, j + 1, strlen(op) - 1); // 获取替换串
            int replaced_cnt = 0;  // 存储替换次数
            replaceString(activeArea, line_no, original, replaced, replaced_cnt);  // 调用替换字符串函数
            cout << Hazel::CYAN << "total replaced counts: " << replaced_cnt << endl
                << "replaced successfully" << Hazel::RESET << endl;
        }
        // 8. 字符串匹配
        else if (op[0] == 'm')
        {
            char match_str[MAXLINESIZE];
            get_str(op, match_str, 1, strlen(op) - 1);  // 活区待匹配字符串
            Pos positions[MAXLINESIZE * ACTIVEMAXLEN] = {0};  // 存储匹配成功的位置
            int matched_cnt = 0;  // 记录匹配成功的个数
            cout << "total matches: " << matched_cnt << endl;
            if (matchString(activeArea, match_str, 1, countLine(activeArea), positions, matched_cnt))  // 匹配活区内所有match
                printPositions(positions, matched_cnt);  // 若匹配成功，则打印所有匹配到的位置
        }
        // 9. 非法输入
        else
        {
            if (count == 1)  // 第一次不是非法操作
            {
                system("cls");
                continue;
            }
            cout << Hazel::RED << "Invalid Input!" << Hazel::RESET << endl;
        }
        cout << Hazel::GREEN << "Press enter to continue..." << Hazel::RESET;
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');  // 判断只接收回车
        system("cls");  // 每操作一次后都清屏
    }

    return 0;
}