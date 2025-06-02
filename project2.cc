/*
 *  CSE 340 Project 2
 *  Combined tasks code
 *  Should compile under C++11
 */

 #include <iostream>
 #include <string>
 #include <vector>
 #include <map>
 #include <set>
 #include <algorithm>
 #include <cctype>
 #include <cstdlib>
 #include "lexer.h"
 
 using namespace std;
 
 // -----------------------------------------------------------
 // The global variables you had for tasks 1–4
 // -----------------------------------------------------------
 LexicalAnalyzer lexer;
 
 vector<string> LHS;
 map<int, vector<string>> RHS;
 vector<string> nullable;
 map<string, vector<string>> first_sets;
 vector<string> appearance_order;
 set<string> seen;
 vector<string> terminal_appearance_order;
 map<string, vector<string>> follow_sets;
 vector<string> original_grammar_lhs;
 map<int, vector<string>> original_grammar;
 map<int, vector<int>> longest_common_prefix_line_toline_length;
 map<int, vector<int>> longest_to_line_length;
 map<int, vector<string>> sorted_grammer_by_longest_prefix;
 vector<string> sorted_lhs;
 vector<int> sorted_line;
 
 // -----------------------------------------------------------
 // Original helper functions for tasks 1–4
 // -----------------------------------------------------------
 void SyntaxError() {
     cout << "SYNTAX ERROR !!!!!!!!!!!!!!" << endl;
     exit(1);
 }
 
 void track_terminal_order(const string &sym) {
     if(find(terminal_appearance_order.begin(), terminal_appearance_order.end(), sym)
        == terminal_appearance_order.end())
     {
         terminal_appearance_order.push_back(sym);
     }
 }
 
 void ReadGrammar() {
     Token token = lexer.GetToken();
     if (token.token_type == HASH) {
         SyntaxError();
     }
     while(token.token_type != HASH) {
         if(token.token_type == ID) {
             LHS.push_back(token.lexeme);
             if(!(lexer.peek(1).token_type == ARROW)) {
                 SyntaxError();
             }
             lexer.GetToken(); // skip arrow
             token = lexer.GetToken();
             if(token.token_type == ID || token.token_type == OR || token.token_type == STAR) {
                 if(token.token_type == STAR || token.token_type == OR) {
                     RHS[LHS.size()].push_back("ε");
                 }
                 while(token.token_type != STAR) {
                     if(token.token_type == ID) {
                         track_terminal_order(token.lexeme);
                         RHS[LHS.size()].push_back(token.lexeme);
                     } else if(token.token_type == OR) {
                         RHS[LHS.size()].push_back("|");
                         if(lexer.peek(1).token_type == STAR || lexer.peek(1).token_type == OR) {
                             RHS[LHS.size()].push_back("ε");
                         }
                     } else {
                         SyntaxError();
                     }
                     token = lexer.GetToken();
                 }
             } else {
                 SyntaxError();
             }
         } else {
             SyntaxError();
         }
         token = lexer.GetToken();
     }
     if(lexer.peek(1).token_type != END_OF_FILE) {
         SyntaxError();
     }
 }
 
 void Task1() {
     vector<string> printed_RHS;
     map<int, vector<string>> collect_LHS;
     for(int i=1; i <= (int)RHS.size(); i++){
         for(size_t j=0; j < RHS[i].size(); j++){
             string s = RHS[i][j];
             if(s!="|" && s!="ε"){
                 if(find(printed_RHS.begin(), printed_RHS.end(), s) == printed_RHS.end()){
                     // If s is not among the LHS, it's a terminal, print it
                     if(find(LHS.begin(), LHS.end(), s) == LHS.end()){
                         cout << s << " ";
                         printed_RHS.push_back(s);
                     } 
                     // else, it's a nonterminal, collect it for later printing
                     else {
                         collect_LHS[i].push_back(s);
                     }
                 }
             }
         }
     }
     vector<string> printed_LHS;
     for(int i=0; i < (int)LHS.size(); i++){
         if(find(printed_LHS.begin(), printed_LHS.end(), LHS[i]) == printed_LHS.end()){
             cout << LHS[i] << " ";
             printed_LHS.push_back(LHS[i]);
         }
         for(size_t j=0; j < collect_LHS[i+1].size(); j++){
             if(find(printed_LHS.begin(), printed_LHS.end(), collect_LHS[i+1][j])==printed_LHS.end()){
                 cout << collect_LHS[i+1][j] << " ";
                 printed_LHS.push_back(collect_LHS[i+1][j]);
             }
         }
     }
 }
 
 void do_nullables(){
     // first pass: any rule that is exactly 'ε'
     for(int i=1; i <= (int)RHS.size(); i++){
         vector<string> &rhs = RHS[i];
         vector<string> current_prod;
         for(int j=0; j <= (int)rhs.size(); j++){
             if(j==(int)rhs.size() || rhs[j] == "|"){
                 if(current_prod.size()==1 && current_prod[0]=="ε"){
                     if(find(nullable.begin(), nullable.end(), LHS[i-1])==nullable.end()){
                         nullable.push_back(LHS[i-1]);
                     }
                 }
                 current_prod.clear();
             } else {
                 current_prod.push_back(rhs[j]);
             }
         }
     }
 
     // fixpoint pass
     bool changed=true;
     while(changed) {
         changed=false;
         for(int i=1; i <= (int)RHS.size(); i++){
             string lhs = LHS[i-1];
             vector<string> &rhs = RHS[i];
             vector<string> current_prod;
             for(int j=0; j <= (int)rhs.size(); j++){
                 if(j==(int)rhs.size() || rhs[j]=="|"){
                     bool all_nullable = true;
                     for(auto &symbol : current_prod){
                         if(symbol=="ε") continue;
                         if(find(nullable.begin(), nullable.end(), symbol)==nullable.end()){
                             all_nullable=false;
                             break;
                         }
                     }
                     if(all_nullable && find(nullable.begin(), nullable.end(), lhs)==nullable.end()){
                         nullable.push_back(lhs);
                         changed=true;
                     }
                     current_prod.clear();
                 } else {
                     current_prod.push_back(rhs[j]);
                 }
             }
         }
     }
 }
 
 void do_order(){
     for(int i=0; i<(int)LHS.size(); i++){
         if(seen.find(LHS[i])==seen.end()){
             appearance_order.push_back(LHS[i]);
             seen.insert(LHS[i]);
         }
         vector<string> &rhs = RHS[i+1];
         for(auto &sym : rhs){
             if(sym!="|" && sym!="ε"
                && find(LHS.begin(), LHS.end(), sym)!=LHS.end()
                && seen.find(sym)==seen.end())
             {
                 appearance_order.push_back(sym);
                 seen.insert(sym);
             }
         }
     }
 }
 
 void Task2() {
     do_nullables();
     do_order();
     if (nullable.empty()){
         cout<<"Nullable = { }"<<endl;
         return;
     }
 
     cout<<"Nullable = { ";
     bool first=true;
     for(auto &sym : appearance_order){
         if(find(nullable.begin(), nullable.end(), sym)!=nullable.end()){
             if(!first) cout<<", ";
             cout<<sym;
             first=false;
         }
     }
     cout<<" }"<<endl;
 }
 
 void do_fristset(){
     // initialize FIRST sets for all symbols
     for(int i=1; i <= (int)RHS.size(); i++){
         first_sets[LHS[i-1]];
         for(size_t j=0; j<RHS[i].size(); j++){
             string s = RHS[i][j];
             if(s!="|" && s!="ε" && find(LHS.begin(), LHS.end(), s)==LHS.end()){
                 if(first_sets.find(s)==first_sets.end()){
                     first_sets[s].push_back(s);
                 }
             }
         }
     }
 
     bool changed=true;
     while(changed){
         changed=false;
         for(int i=1; i <= (int)RHS.size(); i++){
             string lhs = LHS[i-1];
             vector<string> &rhs = RHS[i];
             vector<string> current_prod;
             for(int j=0; j <= (int)rhs.size(); j++){
                 if(j==(int)rhs.size() || rhs[j]=="|"){
                     bool all_nullable=true;
                     for(auto &symbol : current_prod){
                         // Add FIRST(symbol) to FIRST(lhs)
                         for(const auto &x : first_sets[symbol]){
                             if(x!="ε" && find(first_sets[lhs].begin(), first_sets[lhs].end(), x)==first_sets[lhs].end()){
                                 first_sets[lhs].push_back(x);
                                 changed=true;
                             }
                         }
                         if(find(nullable.begin(), nullable.end(), symbol)==nullable.end()){
                             all_nullable=false;
                             break;
                         }
                     }
                     current_prod.clear();
                 } else {
                     current_prod.push_back(rhs[j]);
                 }
             }
         }
     }
 }
 
 void Task3() {
     do_nullables();
     do_fristset();
     do_order();
     vector<string> printed_LHS;
     for(auto &nt : appearance_order){
         if(find(printed_LHS.begin(), printed_LHS.end(), nt)!=printed_LHS.end()){
             continue;
         }
         cout<<"FIRST("<<nt<<") = { ";
         const vector<string> &fv = first_sets.at(nt);
         bool first=true;
         for(auto &t : terminal_appearance_order){
             if(find(fv.begin(), fv.end(), t)!=fv.end()){
                 if(!first) cout<<", ";
                 cout<<t;
                 first=false;
             }
         }
         cout<<" }"<<endl;
         printed_LHS.push_back(nt);
     }
 }
 
 void do_rule_4_5(){
     for(int i=1; i <= (int)RHS.size(); i++){
         int index = (int)RHS[i].size()-1;
         if(index<0) continue;
         string s = RHS[i][index];
         vector<string> temp = first_sets[s];
         for(int j=(int)RHS[i].size()-2; j>=0; j--){
             // If it's a terminal, just update temp
             if(find(LHS.begin(), LHS.end(), RHS[i][j])==LHS.end()){
                 temp=first_sets[RHS[i][j]];
                 continue;
             }
             // If it's '|', skip
             if(s=="|" && j>0){
                 temp=first_sets[RHS[i][j-1]];
                 continue;
             }
             // add temp to follow_sets[RHS[i][j]]
             for(auto &x: temp){
                 follow_sets[RHS[i][j]].push_back(x);
             }
             // if symbol is nullable, add more
             if(find(nullable.begin(), nullable.end(), RHS[i][j])!=nullable.end()){
                 for(auto &xx : first_sets[RHS[i][j]]){
                     temp.push_back(xx);
                 }
             } else {
                 temp=first_sets[RHS[i][j]];
             }
         }
     }
 }
 
 void do_rule_2_3(){
     bool changed=true;
     while(changed){
         changed=false;
         for(int i=1; i <= (int)RHS.size(); i++){
             string lhs = LHS[i-1];
             vector<string> &rhs = RHS[i];
             vector<string> current_prod;
             for(int j=0; j <= (int)rhs.size(); j++){
                 if(j==(int)rhs.size() || rhs[j]=="|"){
                     for(int k=(int)current_prod.size()-1; k>=0; k--){
                         string symbol = current_prod[k];
                         if(find(LHS.begin(), LHS.end(), symbol)==LHS.end()){
                             break;
                         }
                         // add follow(lhs) to follow(symbol)
                         for(auto &xx : follow_sets[lhs]){
                             if(find(follow_sets[symbol].begin(), follow_sets[symbol].end(), xx)==follow_sets[symbol].end()){
                                 follow_sets[symbol].push_back(xx);
                                 changed=true;
                             }
                         }
                         // if symbol is not nullable, stop
                         if(find(nullable.begin(), nullable.end(), symbol)==nullable.end()){
                             break;
                         }
                     }
                     current_prod.clear();
                 } else {
                     current_prod.push_back(rhs[j]);
                 }
             }
         }
     }
 }
 
 void Task4() {
     do_nullables();
     do_fristset();
     // add EOF ($) to follow(startSymbol)
     follow_sets[LHS[0]].push_back("$");
     // ensure each LHS is in follow_sets
     for(int i=0; i<(int)LHS.size(); i++){
         follow_sets[LHS[i]];
     }
     do_rule_4_5();
     do_rule_2_3();
     do_order();
     for(auto &nt : appearance_order){
         auto &S = follow_sets[nt];
         cout<<"FOLLOW("<<nt<<") = { ";
         bool first=true;
         vector<string> printed;
         // $ first if present
         if(find(S.begin(), S.end(), "$")!=S.end()){
             cout<<"$";
             first=false;
             printed.push_back("$");
         }
         // then other terminals in the order they appear
         for(auto &t: terminal_appearance_order){
             if(t=="$") continue;
             if(find(S.begin(), S.end(), t)!=S.end()){
                 if(!first) cout<<", ";
                 cout<<t;
                 first=false;
                 printed.push_back(t);
             }
         }
         cout<<" }"<<endl;
     }
 }
 
 // -----------------------------------------------------------
 // Task 5: Left Factoring
 // -----------------------------------------------------------
 typedef vector<string> Production5;
 typedef vector<Production5> Productions5;
 typedef map<string, Productions5> Grammar5;
 
 // The final factored grammar
 Grammar5 factoredGrammar;
 
 // naming counters
 // -> Maps "base" to how many times we have created a new factor from it.
 map<string,int> originalCounter;
 map<string,int> nonOriginalCounter;
 
 // which original nonterminals have an ε alternative
 set<string> originalHasEpsilon;
 
 // Strip trailing digits. e.g. getBase("A2") => "A"
 static string getBase(const string &nt){
     int pos = (int) nt.size();
     while (pos > 0 && isdigit(nt[pos - 1])) {
         pos--;
     }
     return nt.substr(0, pos);
 }
 
 // does 'nt' end with a digit? e.g. c1, A2
 static bool endsWithDigit(const string &nt){
     if(nt.empty()) return false;
     return isdigit(nt.back())!=0;
 }
 
 // split a single rule’s RHS tokens into separate alternatives
 // each alternative is a Production5
 static vector<Production5> splitProductions5(const vector<string> &rhsTokens){
     vector<Production5> prods;
     Production5 current;
     for(auto &tok : rhsTokens){
         if(tok=="|"){
             prods.push_back(current);
             current.clear();
         } else {
             if(tok=="*") break;
             current.push_back(tok);
         }
     }
     prods.push_back(current);
     return prods;
 }
 
 // convert from your global LHS/RHS => Grammar5
 static Grammar5 convertToGrammar5(){
     Grammar5 G;
     for(int i=1; i<=(int)RHS.size(); i++){
         string lhsSym = LHS[i-1];
         vector<Production5> splitted = splitProductions5(RHS[i]);
         for(auto &p : splitted){
             // if the production is exactly {"ε"}, represent it as empty
             if(p.size()==1 && p[0]=="ε"){
                 p.clear();
                 originalHasEpsilon.insert(lhsSym);
             }
             G[lhsSym].push_back(p);
         }
     }
     return G;
 }
 
 // compute longest common prefix
 static Production5 longestCommonPrefix5(const Productions5 &prods){
     if(prods.empty()) return {};
     Production5 prefix=prods[0];
     for(size_t i=1; i<prods.size(); i++){
         size_t j=0;
         while(j<prefix.size() && j<prods[i].size() && prefix[j]==prods[i][j]){
             j++;
         }
         prefix.resize(j);
     }
     return prefix;
 }
 
 // --------------------------------------------------------------------
 // Updated naming function that always uses X1, X2, X3, ...
 // for each original base "X", in the order you factor it.
 // --------------------------------------------------------------------
 static string newNameForFactor5(const string &nt, bool isOriginal)
 {
     // 1) get a “base” ignoring trailing digits. 
     //    So factoring "A2" also counts as factoring base "A".
     string base = getBase(nt);
 
     if(isOriginal){
         // For an original (or effectively "base") nonterminal
         auto it = originalCounter.find(base);
         if(it == originalCounter.end()) {
             // First factoring => X1
             originalCounter[base] = 1;
             return base + "1";
         } else {
             // subsequent factoring => X2, X3, ...
             originalCounter[base]++;
             return base + to_string(originalCounter[base]);
         }
     } else {
         // For a non-original symbol => we do the same approach,
         // but stored in nonOriginalCounter
         // (still ignoring trailing digits, so factoring "A2" is factoring base "A")
         auto it = nonOriginalCounter.find(base);
         if(it == nonOriginalCounter.end()) {
             // first factoring => X1
             nonOriginalCounter[base] = 1;
             return base + "1";
         } else {
             // next factoring => X2, X3, ...
             nonOriginalCounter[base]++;
             return base + to_string(nonOriginalCounter[base]);
         }
     }
 }
 
 // factor recursively
 static void factorNonterminalRecursively5(const string &nt,
                                           const Productions5 &prods,
                                           bool isOriginal)
 {
     // separate out empty productions
     Productions5 eps, nonEps;
     for(auto &p : prods){
         if(p.empty()) eps.push_back(p);
         else nonEps.push_back(p);
     }
     // group by first symbol
     map<string, Productions5> groups;
     for(auto &p : nonEps){
         groups[p[0]].push_back(p);
     }
     Productions5 result;
     // add empty productions first
     for(auto &p : eps){
         result.push_back(p);
     }
     // now factor each group
     for(auto &kv : groups){
         auto &sameFirst = kv.second;
         if(sameFirst.size()>1){
             Production5 cp = longestCommonPrefix5(sameFirst);
 
             // FACTOR CHECK: Factor if
             // 1) cp is nonempty
             // 2) at least one production in sameFirst is strictly longer than cp
             bool canFactor = false;
             if(!cp.empty()){
                 for(const auto &prod : sameFirst){
                     if(prod.size() > cp.size()){
                         canFactor = true;
                         break;
                     }
                 }
             }
 
             if(canFactor){
                 // factor
                 string newNT = newNameForFactor5(nt, isOriginal);
                 Productions5 newProdsForNewNT;
                 for(auto &prod : sameFirst){
                     Production5 suffix;
                     if(prod.size()>cp.size()){
                         suffix.insert(suffix.end(),
                                       prod.begin()+cp.size(),
                                       prod.end());
                     }
                     newProdsForNewNT.push_back(suffix);
                 }
                 // factor recursively for that new nonterminal
                 factorNonterminalRecursively5(newNT, newProdsForNewNT, false);
 
                 // the factored production for 'nt' includes cp + [newNT]
                 Production5 newProduction=cp;
                 newProduction.push_back(newNT);
                 result.push_back(newProduction);
             } else {
                 // no factoring
                 for(auto &pp : sameFirst){
                     result.push_back(pp);
                 }
             }
         } else {
             // only one production in that group
             result.push_back(sameFirst[0]);
         }
     }
     // sort so empty production first
     sort(result.begin(), result.end(),
          [](const Production5 &a, const Production5 &b){
              if(a.empty() && !b.empty()) return true;
              if(b.empty() && !a.empty()) return false;
              return a<b;
          }
     );
     factoredGrammar[nt]=result;
 }
 
 // print
 static void printGrammar5(const Grammar5 &g){
     vector<pair<string,Production5>> all;
     for(auto &kv : g){
         for(auto &p : kv.second){
             all.push_back({kv.first,p});
         }
     }
     // sort each rule: first by LHS, then by RHS
     sort(all.begin(), all.end(),
          [](const pair<string,Production5> &A, const pair<string,Production5> &B)->bool {
              if(A.first != B.first) return A.first < B.first;
              // empty production first
              if(A.second.empty() && !B.second.empty()) return true;
              if(B.second.empty() && !A.second.empty()) return false;
              return A.second < B.second;
          }
     );
     
     for(auto &rule : all){
         cout << rule.first << " ->";
         if(!rule.second.empty()){
             cout << " ";
             for(size_t i=0; i<rule.second.size(); i++){
                 cout << rule.second[i];
                 if(i+1<rule.second.size()) cout<<" ";
             }
         }
         cout<<" #"<<endl;
     }
 }
 
 // Task5 driver
 void Task5(){
     // clear
     factoredGrammar.clear();
     originalCounter.clear();
     nonOriginalCounter.clear();
     originalHasEpsilon.clear();
     
     // build grammar from global LHS/RHS
     Grammar5 G=convertToGrammar5();
     
     // factor each original nonterminal
     for(auto &kv : G){
         factorNonterminalRecursively5(kv.first, kv.second, true);
     }
     // ensure each original nonterminal is in factoredGrammar
     for(auto &kv : G){
         if(factoredGrammar.find(kv.first)==factoredGrammar.end()){
             factoredGrammar[kv.first]=kv.second;
         }
     }
     // sort each nonterminal's productions
     for(auto &kv : factoredGrammar){
         sort(kv.second.begin(), kv.second.end());
     }
     // print
     printGrammar5(factoredGrammar);
 }
 
 // -----------------------------------------------------------
 // Task6 code (eliminate left recursion)
 // -----------------------------------------------------------
 static Grammar5 eliminateLeftRecursion(Grammar5 G) {
     // gather nonterminals in sorted order
     vector<string> nonterms;
     for(auto &e : G){
         nonterms.push_back(e.first);
     }
     sort(nonterms.begin(), nonterms.end());
     
     for(size_t i=0; i<nonterms.size(); i++){
         string Ai=nonterms[i];
         // for each Aj with j<i
         for(size_t j=0; j<i; j++){
             string Aj=nonterms[j];
             Productions5 newProds;
             for(auto &prod : G[Ai]){
                 if(!prod.empty() && prod[0]==Aj){
                     // expand Ai->Aj gamma => Ai-> delta gamma for each delta in Aj
                     for(auto &delta : G[Aj]){
                         Production5 expanded=delta;
                         if(prod.size()>1){
                             expanded.insert(expanded.end(), prod.begin()+1, prod.end());
                         }
                         newProds.push_back(expanded);
                     }
                 } else {
                     newProds.push_back(prod);
                 }
             }
             G[Ai]=newProds;
         }
         // eliminate immediate left recursion
         Productions5 leftRec, nonLeftRec;
         for(auto &prod : G[Ai]){
             if(!prod.empty() && prod[0]==Ai){
                 leftRec.push_back(prod);
             } else {
                 nonLeftRec.push_back(prod);
             }
         }
         if(!leftRec.empty()){
             // newNT = Ai1, Ai2, ...
             string newNT=Ai+"1";
             int counter=1;
             while(G.find(newNT)!=G.end()){
                 counter++;
                 newNT=Ai+to_string(counter);
             }
             // Ai-> nonLeftRec + newNT
             Productions5 AiNew;
             for(auto &p : nonLeftRec){
                 Production5 appended=p;
                 appended.push_back(newNT);
                 AiNew.push_back(appended);
             }
             G[Ai]=AiNew;
             // newNT-> alpha + newNT | epsilon
             Productions5 alphaSet;
             for(auto &lr : leftRec){
                 Production5 alpha;
                 // drop Ai
                 for(size_t x=1; x<lr.size(); x++){
                     alpha.push_back(lr[x]);
                 }
                 alpha.push_back(newNT);
                 alphaSet.push_back(alpha);
             }
             // add empty production
             Production5 eps; // empty
             alphaSet.push_back(eps);
             G[newNT]=alphaSet;
         }
     }
     return G;
 }
 
 void Task6(){
     // We'll reuse the original LHS,RHS but convert them to a Grammar5 for convenience
     Grammar5 G;
     // convert
     for(int i=1; i<=(int)RHS.size(); i++){
         string lhsSym=LHS[i-1];
         vector<Production5> splitted = splitProductions5(RHS[i]);
         for(auto &p : splitted){
             if(p.size()==1 && p[0]=="ε") {
                 p.clear();
             }
             G[lhsSym].push_back(p);
         }
     }
     // eliminate
     Grammar5 noLeft= eliminateLeftRecursion(G);
     // sort each production
     for(auto &kv : noLeft){
         sort(kv.second.begin(), kv.second.end());
     }
     // print them in sorted order
     vector<pair<string,Production5>> all;
     for(auto &kv : noLeft){
         for(auto &p : kv.second){
             all.push_back({kv.first,p});
         }
     }
     sort(all.begin(), all.end(),
          [](const pair<string,Production5> &A, const pair<string,Production5> &B){
              if(A.first!=B.first) return A.first<B.first;
              if(A.second.empty() && !B.second.empty()) return true;
              if(B.second.empty() && !A.second.empty()) return false;
              return A.second<B.second;
          }
     );
     for(auto &r : all){
         cout<<r.first<<" ->";
         if(!r.second.empty()){
             cout<<" ";
             for(size_t i=0; i<r.second.size(); i++){
                 cout<<r.second[i];
                 if(i+1<r.second.size()) cout<<" ";
             }
         }
         cout<<" #"<<endl;
     }
 }
 
 // -----------------------------------------------------------
 // main
 // -----------------------------------------------------------
 int main(int argc, char* argv[]){
     if(argc<2){
         cout<<"Error: missing argument\n";
         return 1;
     }
     int task=atoi(argv[1]);
     ReadGrammar();
 
     switch(task){
         case 1: Task1(); break;
         case 2: Task2(); break;
         case 3: Task3(); break;
         case 4: Task4(); break;
         case 5: Task5(); break;
         case 6: Task6(); break;
         default:
             cout<<"Error: unrecognized task number "<<task<<"\n";
             break;
     }
     return 0;
 }
 