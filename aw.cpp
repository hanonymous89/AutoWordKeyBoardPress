#include <windows.h>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <unordered_map>
#include <regex>
#include <tuple>
#include <memory>
#include <cmath>
#include <iostream>
#include "rc.h"
namespace h{
        namespace constGlobalData{
            //namespace ...{}
            constexpr auto ENTER="\n";
            constexpr auto SETTING_FILE="setting.ini";
            constexpr auto ICON_NAME=TEXT("WAICON");
            constexpr auto SECTION_POS="POS";
            #define  MAINWINDOWNAME "Mode"
            //constexpr auto mainwindowname....
            constexpr auto MODE_INPUT=MAINWINDOWNAME ":Input";
            constexpr auto MODE_OUTPUT=MAINWINDOWNAME ":Output";
            constexpr auto MAINMENU=TEXT("MAINMENU");
        };
        namespace cast{
        template <class T>
        std::string toString(T str){
            return std::to_string(str);
        }
        std::string toString(char str){
            return std::string(1,str);
        }
        std::string toString(const char *str){
            return std::string(str);
        }
        std::string toString(std::string str){
            return str;
        }
        template <size_t N=0,class T>
        auto toStringFor(std::vector<std::string> &vec,T tuple){
            if constexpr(N<std::tuple_size<T>::value){
                vec.push_back(toString(std::get<N>(tuple)));
                toStringFor<N+1>(vec,tuple);
            }
        }
        template <class... T>
        auto toString(T... str){
            std::tuple<T...> tuple{
                std::make_tuple(str...)
            };
            std::vector<std::string> vec;
            toStringFor(vec,tuple);
            return vec;
        }
        // template <class T> 
        // auto toString(T &str){
        //     std::vector<std::string> vec;
        //     for(auto &obj:str){
        //         vec.push_back(toString(obj));
        //     }
        //     return vec;
        // }
    };
    template <class T>
    inline auto absSub(T left,T right){
        return std::abs(left)-std::abs(right);//sub funcsion
    }
    template <class T>
    inline auto fit(T num,T left,T right){
        return num-absSub(left,right);
    }
    inline auto replaceAll(std::string str,std::string beforeStr,std::string afterStr){
        return std::regex_replace(str,std::regex(beforeStr),afterStr);
    }

    inline std::wstring stringToWstring(std::string str){
        const int BUFSIZE=MultiByteToWideChar(CP_ACP,0,str.c_str(),-1,(wchar_t*)NULL,0);
        std::unique_ptr<wchar_t> wtext(new wchar_t[BUFSIZE]);
        MultiByteToWideChar(CP_ACP,0,str.c_str(),-1,wtext.get(),BUFSIZE);
        return std::wstring(wtext.get(),wtext.get()+BUFSIZE-1);
    }


    inline auto pressKeyAll(std::wstring str){
        constexpr int DATACOUNT=1;
        constexpr int ENTERWIDE=L'\n';
        INPUT input;
        input.type=INPUT_KEYBOARD;
        input.ki.dwExtraInfo=input.ki.time=input.ki.wVk=0;
        for(auto &key:str){
            if(key==ENTERWIDE){
                input.ki.wScan=VK_RETURN;
                input.ki.wVk=MAKEWPARAM(VK_RETURN,0);
            }
            else{
                input.ki.wScan=key;
                input.ki.wVk=0;
            }
            input.ki.dwFlags=KEYEVENTF_UNICODE;
            SendInput(DATACOUNT,&input,sizeof(INPUT));
            input.ki.dwFlags|=KEYEVENTF_KEYUP;
            SendInput(DATACOUNT,&input,sizeof(INPUT));
        }
    }
    template <class T>
    inline T customBool(bool check,T trueT,T falseT)noexcept(false){
        return check?trueT:falseT;
    }
    std::string &linkStr(std::string &str,std::vector<std::string> array){
        for(auto &obj:array){
            str+=obj;
        }
        return str;
    }
    template <class... T>
    std::string &linkStr(std::string &str,T... adds){
        return linkStr(str,cast::toString(adds...));
    }
    
    template <class T>
    inline T clip(T num,T min,T max){
        if(num<min)return min;
        else if(max<num)return max;
        return num;
    }
    inline auto split(std::string str,const std::string cut) noexcept(false){
        std::vector<std::string> data;
        for(auto pos=str.find(cut);pos!=std::string::npos;pos=str.find(cut)){
            data.push_back(str.substr(0,pos));
            str=str.substr(pos+cut.size());
        }
        return data;
    }
    //cast::toStringを使うことでtemplate使わないでいい
    inline auto vecToString(std::vector<std::string> vec,std::string between=""){
        std::string re;//似たような処理が多い
        for(auto &str:vec){
            re+=str+between;
        }
        return re;
    }
    inline std::string strUntil(std::string str,int until,int hash=0,std::string noMatch=""){
        if(0>hash||until+hash>str.size())return noMatch;
        return str.substr(hash,until);
    }

    template <class MapType,class KeyType>
    MapType &editKey(MapType &map,KeyType key,KeyType after)noexcept(false){
        auto node=map.extract(key);
        node.key()=after;
        map.insert(std::move(node));
        return map;
    }
    template <class T>
    bool beMapItem(T &map,typename T::key_type key){
        return map.count(key);
    }
    template <class T>
    void noHitMapValueReplace(T &map,typename T::key_type key,typename T::mapped_type replace=typename T::mapped_type()){
        if(!beMapItem(map,key))map.emplace(key,replace);
    }
    template <class T=std::string>
    inline auto getWindowStr(HWND hwnd){//
        const int BUFSIZE=GetWindowTextLength(hwnd)+1;
        std::unique_ptr<TCHAR> text(new TCHAR[BUFSIZE]);
        GetWindowText(hwnd,text.get(),BUFSIZE);
        return T(text.get(),text.get()+BUFSIZE-1);
    }
    inline auto getListStr(HWND list,int id){//
        const int BUFSIZE=SendMessage(list,LB_GETTEXTLEN,id,0)+1;
        std::unique_ptr<TCHAR> text(new TCHAR[BUFSIZE]);
        SendMessage(list,LB_GETTEXT,id,(LPARAM)text.get());
        return std::string(text.get(),text.get()+BUFSIZE-1);//new decltype(T[0])
    }
    enum modeOperator{
        EQUAL,
        ADD,
        SUB
    };
    inline void windowLong(HWND hwnd,UINT ws,int mode=modeOperator::EQUAL,bool Ex=false){
        UINT style=Ex ? GWL_EXSTYLE:GWL_STYLE;
        switch(mode){
            case modeOperator::ADD:
                ws|=GetWindowLong(hwnd,style);
            break;
            case modeOperator::SUB:
                ws=GetWindowLong(hwnd,style)&~ws;
            break;
        }
        SetWindowLong(hwnd,style,ws);
    }
    template <class getT>
    class mapManager{
        public:
        template <class MapT>
        typename getT::mapped_type &get(MapT &map,std::vector<typename getT::key_type> keys){
            std::string key=keys[0];
            keys.erase(keys.begin());
            noHitMapValueReplace(map,key);
            return get(map.at(key),keys);
        }
        typename getT::mapped_type &get(getT &map,std::vector<typename getT::key_type> keys){
            noHitMapValueReplace(map,keys[0]);
            return map.at(keys[0]);
        }

        template <class MapT,class... KeyT>
        typename getT::mapped_type &get(MapT &map,KeyT... keys){
            return get(map,std::vector<typename getT::key_type>{keys...});///
        }
    };
    inline auto StrToInt(std::string str,int dataCount){
        std::vector<int> list;
        list.resize(dataCount);
        std::smatch m{};
        for(auto &item:list){
            std::regex_search(str,m,std::regex(R"(\d+)"));
            item=std::stoi("0"+m[0].str());
            str=m.suffix();
        }
        return list;
    }
    class File{
        private:
        std::string name,
                    content;
        public:
        inline File(const std::string name)noexcept(true):name(name){
            read();
        }
        inline auto &setName(const std::string name)noexcept(true){
            this->name=name;
            return *this;
        }
        inline auto &getContent()const noexcept(true) {
            return content;
        }
        inline File &read() noexcept(false){
            std::fstream file(name);
            if(file.fail())return *this;
            content=std::string(std::istreambuf_iterator<char>(file),std::istreambuf_iterator<char>());
            return *this;
        }
        inline auto write(std::string str,bool reset=false){
            std::ofstream file(name,customBool(reset,std::ios_base::trunc,std::ios_base::app));
            file<<str;
            file.close();
            return *this;
        }
        inline  auto &replace(const std::string data,const bool reset=false)const noexcept(false){
            std::ofstream file(name,customBool<std::ios_base::openmode>(reset,std::ios_base::out,std::ios_base::app));
            if(file.fail())return *this;
            file<<data;
            file.close();
            return *this;
        }
    };
    namespace INIT{
        using keyT=std::unordered_map<std::string,std::string>;
        using sectionT=std::unordered_map<std::string,keyT>; 
    };
    class INI : private File{
        private:
        static constexpr auto SECTIONLEFT='[',SECTIONRIGHT=']',EQUAL='=';
        INIT::sectionT data;
        inline const INI analysis() noexcept(false){
            auto lines=split(read().getContent(),constGlobalData::ENTER);
            for(auto &line:lines){
                static std::string section;
                if(line[0]==SECTIONLEFT&&line[line.size()-1]==SECTIONRIGHT){
                    section=strUntil(line,line.size()-2,1);
                    continue;
                }
                auto equalPos=line.find(EQUAL);
                data[section][strUntil(line,equalPos)]=line.substr(equalPos+sizeof(EQUAL));
            }
            return *this;
        }
        public:
        inline INI(const std::string name)noexcept(true):File(name){
            analysis();
        }
        inline  auto changeFile(std::string name) noexcept(true) {
            setName(name);
            analysis();
            return *this;
        }
        template<class T,class... KeyT>
        inline typename T::mapped_type getData(KeyT... keys)noexcept(true){
            return  mapManager<T>().get(data,std::vector<std::string>{keys...});
        }
        template <class T,class... KeyT>
        inline auto deleteData(std::string delKey,KeyT... keys)noexcept(true){
            mapManager<T>().get(data,keys...).erase(delKey);
            return *this;
        }
        // template <class T,class... KeyT>
        // inline auto editKey(std::string afterKey,KeyT... keys)noexcept(true){
        //     std::vector<std::string> Keys{FILE,keys...};
        //     h::editKey(mapManager<std::unordered_map<std::string,T> >().get(data,Keys),Keys[Keys.size()-1],afterKey);
        //     return *this;
        // }
        //you should use template(mapManager)
        inline auto editValue(const std::string section,const std::string key,std::string after)noexcept(true){
            noHitMapValueReplace(data,section);
            noHitMapValueReplace(data[section],key);
            data[section][key]=after;
            return *this;
        }
        inline  auto makeString(){
            std::string textData;
            for(auto &[section,line]:data){
                linkStr(textData,SECTIONLEFT,section,SECTIONRIGHT,constGlobalData::ENTER);//似ている処理
                for(auto &[key,value]:line){
                    linkStr(textData,key,EQUAL,value,constGlobalData::ENTER);
                }
            }
            return textData;
        }
        inline const auto save(){
            replace(makeString(),true);
            return *this;
        }
    };
    template <class T>
    struct RGB{
        static constexpr int R=0,G=R+1,B=G+1;
        T rgb[3];
    };
    class RGBManager{
        private:
        RGB<int> rgb;
        public:
        inline RGBManager(RGB<int> rgb)noexcept(true):rgb(rgb){

        }
        inline RGBManager(){

        }
        inline auto set(std::string str){
            for(auto obj:StrToInt(str,3)){
                static int i=0;
                rgb.rgb[i]=obj;
            }
            // for(auto &obj:rgb.rgb){//strtoint
            //     std::smatch m{};
            //     std::regex_search(str,m,std::regex(R"(\d+)"));
            //     obj=std::stoi("0"+m[0].str());
            //     str=m.suffix();
            // }
            return *this;
        }
        inline auto &get(){
            return rgb;
        }
        inline auto makeString(std::string between=" "){
            std::string re;
            for(auto &obj:rgb.rgb){
                linkStr(re,obj,between);
            }
            //linkstr("",rgb.rgb,between);
            return re;
        }
    };
    inline auto baseStyle(WNDPROC wndproc,LPCSTR name){
        /*static? i should make closure?*/WNDCLASS winc;
        winc.style=CS_VREDRAW|CS_HREDRAW;
        winc.cbClsExtra=winc.cbWndExtra=0;
        winc.hInstance=(HINSTANCE)GetModuleHandle(0);
        winc.hIcon=LoadIcon(winc.hInstance,h::constGlobalData::ICON_NAME);
        winc.hCursor=LoadCursor(NULL,IDC_ARROW);
        winc.lpszMenuName=NULL;
        winc.hbrBackground=(HBRUSH)GetStockObject(BLACK_BRUSH);
        winc.lpfnWndProc=wndproc;
        winc.lpszClassName=name;
        return RegisterClass(&winc);
    }

};
BOOL CALLBACK enumChildSave(HWND hwnd,LPARAM lp){
    WINDOWPLACEMENT wndPlace;
    wndPlace.length=sizeof(WINDOWPLACEMENT);
    GetWindowPlacement(hwnd,&wndPlace);
    h::INI(h::constGlobalData::SETTING_FILE)
    .editValue(h::constGlobalData::SECTION_POS,h::getWindowStr(hwnd),h::vecToString(h::cast::toString(wndPlace.rcNormalPosition.left,wndPlace.rcNormalPosition.top,wndPlace.rcNormalPosition.right,wndPlace.rcNormalPosition.bottom)," "))
    .save();
    return TRUE;
}
BOOL CALLBACK enumChildLoad(HWND hwnd,LPARAM lp){
    auto rect=h::StrToInt(h::INI(h::constGlobalData::SETTING_FILE).getData<h::INIT::keyT>(h::constGlobalData::SECTION_POS,h::getWindowStr(hwnd)),4);
    MoveWindow(hwnd,rect[0],rect[1],rect[2],rect[3],TRUE);
    return TRUE;
}
LRESULT CALLBACK WndProc(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp){
    constexpr int MSG1=1,
                  MSG2=MSG1+1,
                  MSG3=MSG2+1;
    constexpr auto CELEND="[CELEND]",
                   REPLACE_ENTER="[ENTER]";
    constexpr auto SECTION_STRING="STRING";
    constexpr auto KEY_WORDLIST="wordList";
    constexpr auto HELPHTML="EasyHelp.html";
    static HWND list,
                edit;
    static OPENFILENAME ofn{0};
    static TCHAR path[MAX_PATH];
    static int count=0;
    std::string str;
    RECT rect;
    switch(msg){
        case WM_DESTROY:
            for(int i=SendMessage(list,LB_GETCOUNT,0,0)-1;i>=0;--i){
                str+=h::getListStr(list,i)+CELEND;
            }
            GetWindowRect(hwnd,&rect);
            h::INI(h::constGlobalData::SETTING_FILE).editValue(SECTION_STRING,KEY_WORDLIST,str)
            .editValue(h::constGlobalData::SECTION_POS,MAINWINDOWNAME,h::vecToString(h::cast::toString(rect.left,rect.top,rect.right-rect.left,rect.bottom-rect.top)," "))
            .save();
            EnumChildWindows(hwnd,enumChildSave,0);
            --count;
            if(0>=count){
                PostQuitMessage(0);
            }
            
        break;
        case WM_CREATE:
            ++count;
            SendMessage(CreateWindow(TEXT("BUTTON"),TEXT("Add"),WS_VISIBLE|WS_OVERLAPPEDWINDOW|WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|BS_MULTILINE,0,0,0,0,hwnd,(HMENU)MSG1,LPCREATESTRUCT(lp)->hInstance,NULL),WM_SETICON,ICON_BIG,(LPARAM)LoadIcon(LPCREATESTRUCT(lp)->hInstance,h::constGlobalData::ICON_NAME));
            SendMessage(CreateWindow(TEXT("BUTTON"),TEXT("Sub"),WS_VISIBLE|WS_OVERLAPPEDWINDOW|WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|BS_MULTILINE,0,0,0,0,hwnd,(HMENU)MSG3,LPCREATESTRUCT(lp)->hInstance,NULL),WM_SETICON,ICON_BIG,(LPARAM)LoadIcon(LPCREATESTRUCT(lp)->hInstance,h::constGlobalData::ICON_NAME));
            list=CreateWindow(TEXT("LISTBOX"),TEXT(KEY_WORDLIST),WS_HSCROLL|WS_VSCROLL|LBS_NOTIFY |WS_VISIBLE|WS_OVERLAPPEDWINDOW|WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS,0,0,0,0,hwnd,(HMENU)MSG2,(LPCREATESTRUCT(lp))->hInstance,NULL);
            SendMessage(list,WM_SETICON,ICON_BIG,(LPARAM)LoadIcon(LPCREATESTRUCT(lp)->hInstance,h::constGlobalData::ICON_NAME));
            edit=CreateWindow(TEXT("EDIT"),TEXT(""),WS_SIZEBOX|WS_VISIBLE|WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|ES_MULTILINE|ES_WANTRETURN,0,0,100,100,hwnd,NULL,LPCREATESTRUCT(lp)->hInstance,NULL);
            SendMessage(edit,WM_SETICON,ICON_BIG,(LPARAM)LoadIcon(LPCREATESTRUCT(lp)->hInstance,h::constGlobalData::ICON_NAME));
            ofn.lStructSize=sizeof(OPENFILENAME);
            ofn.hwndOwner=hwnd;
            ofn.lpstrFilter=TEXT("All files {*.*}\0*.*\0\0");
            ofn.lpstrCustomFilter=0;
            ofn.lpstrFile=path;
            ofn.nMaxFile=MAX_PATH;
            ofn.Flags=OFN_NOVALIDATE;
            if(!std::filesystem::exists(h::constGlobalData::SETTING_FILE)){
                std::ofstream(h::constGlobalData::SETTING_FILE);
            }
            for(auto word:h::split(h::INI(h::constGlobalData::SETTING_FILE).getData<h::INIT::keyT>(SECTION_STRING,KEY_WORDLIST),CELEND)){
                SendMessage(list,LB_ADDSTRING,0,(LPARAM)word.c_str());
            }
            {
                auto re=h::StrToInt(h::INI(h::constGlobalData::SETTING_FILE).getData<h::INIT::keyT>(h::constGlobalData::SECTION_POS,MAINWINDOWNAME),4);
                MoveWindow(hwnd,re[0],re[1],re[2],re[3],TRUE);
                EnumChildWindows(hwnd,enumChildLoad,0);
            }
        break;
        case WM_RBUTTONDOWN:
            h::windowLong(hwnd,WS_EX_NOACTIVATE,h::customBool(h::getWindowStr(hwnd).compare(h::constGlobalData::MODE_INPUT),h::modeOperator::SUB,h::modeOperator::ADD),true);
            SetWindowText(hwnd,h::customBool(h::getWindowStr(hwnd).compare(h::constGlobalData::MODE_INPUT),TEXT(h::constGlobalData::MODE_INPUT),TEXT(h::constGlobalData::MODE_OUTPUT)));
        break;
        case WM_COMMAND:
            switch(LOWORD(wp)){
                case MSG1:
                    SendMessage(list,LB_ADDSTRING,SendMessage(list,LB_GETCOUNT,0,0),(LPARAM)h::replaceAll(h::getWindowStr(edit),"\r\n",REPLACE_ENTER).c_str());
                break;
                case MSG3:
                    if(SendMessage(list,LB_GETCURSEL,0,0)==LB_ERR)break;
                    SendMessage(list,LB_DELETESTRING,SendMessage(list,LB_GETCURSEL,0,0),0);
                break;
                case MSG2:
                    if(HIWORD(wp)==LBN_DBLCLK){
                        h::pressKeyAll(h::stringToWstring(h::replaceAll(h::getListStr(list,SendMessage(list,LB_GETCURSEL,0,0)),REPLACE_ENTER,h::constGlobalData::ENTER)));
                    }
                    
                break;
                case FILEOPEN:
                    if(!GetOpenFileName(&ofn))break;
                    if(MessageBox(hwnd,TEXT("reset?"),TEXT("Question"),MB_ICONQUESTION|MB_YESNO)==IDYES){
                        SendMessage(list,LB_RESETCONTENT,0,0);
                    }
                    for(auto item:h::split(h::File(path).read().getContent(),CELEND)){
                        SendMessage(list,LB_ADDSTRING,0,(LPARAM)item.c_str());
                    }
                break;
                case FILESAVE:
                    if(!GetOpenFileName(&ofn))break;
                    for(int i=SendMessage(list,LB_GETCOUNT,0,0)-1;i>=0;--i){
                        str+=h::getListStr(list,i)+CELEND;
                    }
                    h::File(path).write(str,MessageBox(hwnd,TEXT("reset?"),TEXT("Question"),MB_ICONQUESTION|MB_YESNO)==IDYES);
                break;
                case SHOWHELP:
                    if(!std::filesystem::exists(HELPHTML))h::File(HELPHTML).write(
                    "<!DOCTYPE html><html><head><meta charset=\"utf-8\"><title>EasyHelp</title></head><body><h1>モードについて</h1><div>ウィンドウ内を右クリック:モード変更</div><div>ウィンドウのタイトルバーを見るとモードが書いてある</div><div>Mode:Inputの場合は本ソフトウェアにテキストを書き込めます</div><div>Mode:Outputの場合は入力したい場所にフォーカスをあて入力します</div><div>※入力については後述</div><h1>本ソフトウェアそれぞれのウィンドウについて</h1><div>タイトルバーの無いウィンドウがテキストボックスになります</div><div>※テキストボックスは枠の色が変わっていることろをドラッグするとサイズ変更と疑似的な移動ができます</div><div>\"wordList\"は自動入力するデータを保存し自動入力に使われます</div><div>\"Add\"はテキストボックスの内容をリストに追加します</div><div>\"Sub\"は選択されているリストのデータを削除します</div><h1>入力について</h1><div>モードを\"Mode:Output\"に変更し入力したい場所にフォーカスを当てリストをダブルクリックしてください</div><h1>ファイルの読み取りとセーブについて</h1><div>本ソフトウェアのタイトルバー付近にある\"FILE (F)\"について説明します</div><div>\"FILE(F)\"の\"FILE OPEN\"でファイル選択ウィンドウが開きます</div><div>※ファイル選択については省略</div><div>開いたファイルの[CELEND]までを一つのデータとしリストに表示します</div><div>その際にメッセージボックスが開いてリストをリセットするか聞かれるので「はい」を選択とリストがリセットされ「いいえ」を選択するとそのまま追加されます</div><div>\"FILE(F)\"の\"FILE SAVE\"で同じようにファイル選択ウィンドウが開きます</div><div>開くと先ほどと同じようなファイルをリセットするかファイルの最後に追加するかを聞かれます</div><div>※選択後の説明は略</div><h1>設定ファイルについて</h1><div>本ソフトウェアと同じフォルダ（ディレクトリ）内にあるsetting(.ini)について説明します</div><div>このファイルは起動時に同じフォルダ内に見つからない場合新しく作成されます</div><div>それぞれのデータがどのように機能するかは多すぎるので書きません</div></body></html>"
                    );
                    system(std::filesystem::absolute(HELPHTML).string().c_str());
                break;
                case NEWWINDOW:
                    SendMessage(CreateWindowEx(WS_EX_TOPMOST,TEXT("MAIN"),TEXT(h::constGlobalData::MODE_INPUT),WS_VISIBLE|WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,NULL,LoadMenu((HINSTANCE)GetModuleHandle(0),h::constGlobalData::MAINMENU),(HINSTANCE)GetModuleHandle(0),NULL),WM_SETICON,ICON_BIG,(LPARAM)LoadIcon((HINSTANCE)GetModuleHandle(0),h::constGlobalData::ICON_NAME));
                break;
            }
        break;
    }
    return DefWindowProc(hwnd,msg,wp,lp);
}
int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,PSTR lpCmdLine,int nCmdShow){
    MSG msg;
    h::baseStyle(WndProc,"MAIN");
    SendMessage(CreateWindowEx(WS_EX_TOPMOST,TEXT("MAIN"),TEXT(h::constGlobalData::MODE_INPUT),WS_VISIBLE|WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,NULL,LoadMenu(hInstance,h::constGlobalData::MAINMENU),hInstance,NULL),WM_SETICON,ICON_BIG,(LPARAM)LoadIcon(hInstance,h::constGlobalData::ICON_NAME));
    while(GetMessage(&msg,NULL,0,0)){
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return msg.wParam;
}
