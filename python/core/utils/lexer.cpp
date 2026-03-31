#include "lexer.h"

Lex::Lexer::Lexer(QObject *parent) : QObject(parent)
{

}

std::vector<Lex::Token> Lex::Lexer::scanTokens(const std::string& text)
{
    std::vector<Token> result;
    auto lines = splitByLineBreak(text);
    for (size_t i = 0; i < lines.size(); ++i) {
        size_t first = lines[i].find_first_not_of(" \t\n\r");
        if (first == std::string::npos) {
            continue;
        }
        auto tokens = scanPartToken(lines[i], i + 1);
        for (auto& token : tokens) {
            result.push_back(token);
        }
    }
    // 补全DEDENT
    while (indentStack.back() != 0) {
        result.emplace_back(TokenType::DEDENT, "", int(lines.size()));
        indentStack.pop_back();
    }
    result.emplace_back(TokenType::EOF_TOKEN, "", int(lines.size() + 1));
    QVector<Token> tokens;
    tokens.reserve(result.size());
    for (const auto& t : result) {
        tokens.append(t);
    }
    emit finished(tokens);
    return result;
}

std::vector<std::string> Lex::Lexer::splitByLineBreak(const std::string& text)
{
    std::vector<std::string> result;
    size_t start = 0;
    size_t textLen = text.size();

    for (size_t i = 0; i < textLen; ++i) {
        // 处理 Windows 换行符 \r\n
        if (text[i] == '\r' && i + 1 < textLen && text[i + 1] == '\n') {
            // 截取 [start, i)
            result.push_back(text.substr(start, i - start));
            start = i + 2;  // 跳过 \r\n
            i++;
        }
        // 处理 Unix 换行符 \n
        else if (text[i] == '\n') {
            // 截取 [start, i)
            result.push_back(text.substr(start, i - start));
            start = i + 1;  // 跳过 \n
        }
    }

    // 补充最后一行
    if (start < textLen) {
        result.push_back(text.substr(start));
    }

    return result;
}

std::vector<Lex::Token> Lex::Lexer::scanPartToken(const std::string& line, int lineNum)
{
    std::vector<Token> tokens;
    size_t current = 0; // 这个是当前的指针
    size_t lineLen = line.size(); // 这个是这一行的长度
    int current_indent = 0;

    // 应当在这里统计缩进
    int cnt = 0;
    while (current < lineLen && (line[current] == '\t' || line[current] == ' ')) {
        if (line[current] == '\t')
            cnt += 4;
        else
            cnt++;
        current++;
    }
    int indentCount = cnt / 4;
    if (indentCount > indentStack.back() && bracketNesting == 0) {
        tokens.emplace_back(TokenType::INDENT, "", lineNum);
        indentStack.push_back(indentCount);
        current_indent = indentCount;
    }
    else while (indentCount < indentStack.back() && bracketNesting == 0) {
        tokens.emplace_back(TokenType::DEDENT, "", lineNum);
        indentStack.pop_back();
        current_indent = indentStack.back();
    }

    // 思路是逐个读取逐个解析
    while (current < lineLen) {
        char c = line[current]; // 当前字符

        // 首先跳过空格
        if (c == ' ' || c == '\t') {
            current += 1;
            continue;
        }
        // 可以作为函数或变量的开头字符
        else if (isalpha(c) || c == '_') {
            // 从这里开始寻找整个字段, 读到不是字母，数字，下划线为止
            size_t start = current;
            while (current < lineLen && (isalnum(line[current]) || line[current] == '_'))
            {
                current++;
            }
            // 从start开始截取到current过
            std::string lexeme(line.substr(start, current - start));
            // 判断是否为关键字
            auto kwIt = KEYWORDS.find(lexeme);
            if (kwIt != KEYWORDS.end()) {
                tokens.emplace_back(kwIt->second, lexeme, lineNum);
            }
            else {
                tokens.emplace_back(TokenType::IDENTIFIER, lexeme, lineNum);
            }
        }
        // 识别数字
        else if (isdigit(c)) // 必须以数字开头
        {
            size_t start = current;
            // 1. 扫描整数部分
            while (current < lineLen && isdigit(line[current])) {
                current++;
            }

            bool hasDot = false;  // 是否出现小数点
            size_t dotAfterPos = current;  // 小数点后的起始位置

            // 2. 检查是否有小数点
            if (current < lineLen && line[current] == '.') {
                current++;
                hasDot = true;
                dotAfterPos = current;  // 记录小数点后第一个字符的索引
            }

            // 出现小数点，才扫描小数部分
            if (hasDot) {
                while (current < lineLen && isdigit(line[current])) {
                    current++;
                }
            }

            // 4. 截取lexeme
            std::string lexeme = line.substr(start, current - start);

            if (hasDot && (dotAfterPos == current)) {
                tokens.emplace_back(TokenType::ERROR, lexeme, lineNum);
            }
            else {
                if (hasDot)
                    tokens.emplace_back(TokenType::FLOAT, lexeme, lineNum);
                else
                    tokens.emplace_back(TokenType::INT, lexeme, lineNum);
            }
        }
        else if (c == '"' || c == '\'') // 字符串解析
        {
            char quote = c;  // 记录当前包裹符
            size_t start = current + 1;  // 跳过开头引号
            current++;  // 移动指针到字符串内容
            // 扫描到闭合引号
            while (current < lineLen && line[current] != quote) {
                current++;
            }
            // 检查是否正常闭合
            if (current >= lineLen) {
                // 未闭合字符串，生成错误Token
                tokens.emplace_back(TokenType::ERROR, "Unclosed string", lineNum);
            }
            else {
                // 截取lexme
                std::string lexeme = line.substr(start, current - start);
                tokens.emplace_back(TokenType::STRING, lexeme, lineNum);
                current++;  // 跳过闭合引号
            }
        }
        // 匹配运算符
        else if (c == '=' ||
                 c == '>' ||
                 c == '<' ||
                 c == '&' ||
                 c == '|' ||
                 c == '!') {
            // peek一下看看下一个是不是可以组成二元的运算符
            if (current + 1 < lineLen) {
                char nextC = line[current + 1];
                if (c == '=' && nextC == '=') {  // ==
                    tokens.emplace_back(TokenType::EQ, "==", lineNum);
                    current += 2;  // 跳过两个字符
                    continue;
                }
                else if (c == '!' && nextC == '=') {  // !=
                    tokens.emplace_back(TokenType::NEQ, "!=", lineNum);
                    current += 2;
                    continue;
                }
                else if (c == '<' && nextC == '=') {  // <=
                    tokens.emplace_back(TokenType::LTE, "<=", lineNum);
                    current += 2;
                    continue;
                }
                else if (c == '>' && nextC == '=') {  // >=
                    tokens.emplace_back(TokenType::GTE, ">=", lineNum);
                    current += 2;
                    continue;
                }
            }
            if (c == '=') {  // =
                tokens.emplace_back(TokenType::ASSIGN, "=", lineNum);
            }
            else if (c == '<') {  // <
                tokens.emplace_back(TokenType::LT, "<", lineNum);
            }
            else if (c == '>') {  // >
                tokens.emplace_back(TokenType::GT, ">", lineNum);
            }
            else if (c == '!') {
                tokens.emplace_back(TokenType::NOT, "!", lineNum);
            }
            else {  // & 或 |（单独出现，错误）
                tokens.emplace_back(TokenType::ERROR, "Invalid operator: " + std::string(1, c), lineNum);
            }
            current++;  // 跳过当前字符
        }
        else if (c == '+' || c == '-' || c == '*' || c == '/' || c == '%') {
            if (c == '+') tokens.emplace_back(TokenType::PLUS, "+", lineNum);
            else if (c == '-') tokens.emplace_back(TokenType::MINUS, "-", lineNum);
            else if (c == '*') tokens.emplace_back(TokenType::STAR, "*", lineNum);
            else if (c == '/') tokens.emplace_back(TokenType::SLASH, "/", lineNum);
            else if (c == '%') tokens.emplace_back(TokenType::MOD, "%", lineNum);
            current++;
        }
        else if (c == '(') {  // (
            tokens.emplace_back(TokenType::LPAREN, "(", lineNum);
            bracketNesting++;
            current++;
        }
        else if (c == ')') {  // )
            tokens.emplace_back(TokenType::RPAREN, ")", lineNum);
            bracketNesting--;
            current++;
        }
        else if (c == '[') {  // {
            tokens.emplace_back(TokenType::LBRACKET, "[", lineNum);
            bracketNesting++;
            current++;
            }
        else if (c == ']') {  // }
            tokens.emplace_back(TokenType::RBRACKET, "]", lineNum);
            bracketNesting--;
            current++;
        }
        else if (c == '{') {
            tokens.emplace_back(TokenType::LBRACE, "{", lineNum);
            bracketNesting++;
            current++;
        }
        else if (c == '}') {
            tokens.emplace_back(TokenType::RBRACE, "}", lineNum);
            bracketNesting--;
            current++;
        }
        else if (c == '#') { // 注释
            while (current < lineLen && line[current] != '\n')
                current++;
        }
        else if (c == ',') {
            tokens.emplace_back(TokenType::COMMA, ",", lineNum);
            current++;
        }
        else if (c == ':') {
            tokens.emplace_back(TokenType::COLON, ":", lineNum);
            current++;
        }
        else if (c == '.') {
            tokens.emplace_back(TokenType::DOT, ".", lineNum);
            current++;
        }
        // 未知字符
        else {
            tokens.emplace_back(TokenType::ERROR, "Unexpected character: " + std::string(1, c), lineNum);
            current++;  // 跳过错误字符，继续解析后续内容
        }
    }
    // tokens.emplace_back(TokenType::EOF_TOKEN, "", lineNum);
    return tokens;
}
