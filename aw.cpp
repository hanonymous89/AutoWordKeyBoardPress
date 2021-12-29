#include <windows.h>
#include <string>
#include <regex>
#include <vector>
#include <filesystem>
#include <fstream>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <memory>
#include <limits>
#include <bitset>
#include <functional>
#include "rc.h"
namespace h{
        
        template <class BT,class CT>
        class ObjectManager{
            protected:
            BT base;
            CT created;
            virtual void cr(BT base)=0;
            virtual void dr()=0;
            public:
            ObjectManager(){}
            ObjectManager(BT base):base(base){}
            virtual ~ObjectManager(){}
            virtual BT &getBase()=0;
            virtual CT &getCreated()=0;
            virtual void reset(BT base)=0;
            
        };//hdc!!!
        class colorManager:private ObjectManager<COLORREF,HBRUSH>{
            private:
            void cr(COLORREF base) override{
                this->base=base;
                created=CreateSolidBrush(base);
            }
            void dr(){
                if(created==NULL)return;
                DeleteObject(created);                
            }
            public:
            colorManager(){}
            colorManager(COLORREF base):ObjectManager(base){
                cr(base);
            }
            COLORREF &getBase()override{
                return base;
            }
            HBRUSH &getCreated()override{
                return created;
            }
            ~colorManager() override{
                dr();
            }
            void reset(COLORREF base)override{
                dr();
                cr(base);
            }
        };
        class fontManager:private ObjectManager<std::string,HFONT>{
            private:
            int height;
            void cr(std::string base) override{
                this->base=base;
                created=CreateFont(height,0,0,0,FW_REGULAR,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH,base.c_str());
            }
            void dr(){
                if(created==NULL)return;
                DeleteObject(created);                
            }
            public:
            fontManager(){}
            fontManager(std::string base):ObjectManager(base){
                cr(base);
            }
            std::string &getBase()override{
                return base;
            }
            HFONT &getCreated()override{
                return created;
            }
            ~fontManager() override{
                dr();
            }
            void reset(std::string base)override{
                dr();
                cr(base);
            }
            auto &setHeight(decltype(height) height){
                this->height=height;
                reset(base);
                return *this;
            }
        };
        namespace constGlobalData{
            #define  MAINWINDOWNAME "Mode"
            constexpr auto ICON_NAME=TEXT("WAICON"),
                           MAINMENU=TEXT("MAINMENU");
            constexpr auto ENTER="\n",
                           SETTING_FILE="setting.ini",
                           SECTION_POS="POS",
                           MODE_INPUT=MAINWINDOWNAME ":Input",
                           MODE_OUTPUT=MAINWINDOWNAME ":Output",
                           SECTION_SHOW="SHOW",
                           KEY_ALPHA="alpha",
                           KEY_BK_COLOR="bk",
                           KEY_BORDER_COLOR="border",
                           KEY_FONT="font",
                           SCROLL_WINDOW="SCROLL",
                           SIMPLEBTN_WINDOW="SIMPLEBTN",
                           SIMPLELIST_WINDOW="SIMPLELIST",
                           SETTING_WINDOW="SETTING",
                           CELEND="[CELEND]",
                           SECTION_STRING="STRING",
                           KEY_WORDLIST="wordList",
                           HELPHTML="EasyHelp.html";

            constexpr auto MOUSE_UPDATA_COUNT=10;
            enum SCROLL{
                GET_SCROLL=100,
                SET,
                SET_END,
                CHANGE,
            };       
            enum BTN{
                SET_BTN=200
            };           
            enum LIST{
                SET_LIST=300,
                PUSH,
                GET_SELECT_INDEX,
                DELETE_ITEM,
                GET_ITEM,
                GET_OBJ
            };     
            enum BTN_LIST{
                SET_BTN_LiST,
            };        
            enum MENU{
                SET_MENU=400,

            };
        };

        namespace global{
            std::vector<HWND> hwnds;
            std::vector<std::string> *vecStr;
            colorManager borderBrush,
                         bkBrush;
            fontManager font;
        };
        template <class T>
        bool beMapItem(T &map,typename T::key_type key){
            return map.count(key);
        }
        template <class T>
        inline T &noHitMapValueReplace(T &map, typename T::key_type key, typename T::mapped_type replace=typename T::mapped_type()){
            if(!beMapItem(map,key))map.emplace(key,replace);
            return map;
        }
        template <class T>
        inline T returnNoHitMapValueReplace(T map, typename T::key_type key, typename T::mapped_type replace=typename T::mapped_type()){
            if(!beMapItem(map,key))map.emplace(key,replace);
            return map;
        }
        inline std::string strUntil(std::string str,int until,int hash=0,std::string noMatch=""){
        if(0>hash||until+hash>str.size())return noMatch;
        return str.substr(hash,until);
        }
        namespace cast{
        template <class T>
        inline std::string toString(const T str)noexcept(true){
            return std::to_string(str);
        }
        inline std::string toString(const char str)noexcept(true){
            return std::string(1,str);
        }
        inline std::string toString(const char *str)noexcept(true){
            return std::string(str);
        }
        inline std::string toString(const std::string str)noexcept(true){
            return str;
        }
        template <size_t N=0,class T>
        inline auto toStringFor(std::vector<std::string> &vec,const T tuple)noexcept(true){
            if constexpr(N<std::tuple_size<T>::value){
                vec.push_back(toString(std::get<N>(tuple)));
                toStringFor<N+1>(vec,tuple);
            }
        }
        template <class... T>
        inline auto toString(const T... str)noexcept(true){
            std::tuple<T...> tuple{
                std::make_tuple(str...)
            };
            std::vector<std::string> vec;
            toStringFor(vec,tuple);
            return vec;
        }
        inline int customStoi(const std::string str) noexcept(false){
            return std::stoi("0"+str);
        }
        inline int toInt(const std::string &str)noexcept(false){
            const int maxLength=toString(std::numeric_limits<int>::max()).size()-1;
            if(str.size()<=maxLength)return customStoi(str);
            return customStoi(strUntil(str,maxLength));
        }
    };
    inline auto replaceAll(const std::string str,const std::string beforeStr,const std::string afterStr)noexcept(true){
        return std::regex_replace(str,std::regex(beforeStr),afterStr);
    }
    inline std::wstring stringToWstring(const std::string str)noexcept(true){
        const int BUFSIZE=MultiByteToWideChar(CP_ACP,0,str.c_str(),-1,(wchar_t*)NULL,0);
        std::unique_ptr<wchar_t> wtext(new wchar_t[BUFSIZE]);
        MultiByteToWideChar(CP_ACP,0,str.c_str(),-1,wtext.get(),BUFSIZE);
        return std::wstring(wtext.get(),wtext.get()+BUFSIZE-1);
    }
    inline auto pressKeyAll(const std::wstring str)noexcept(true){
        constexpr int DATACOUNT=1;
        constexpr int ENTERWIDE=L'\n';
        INPUT input;
        input.type=INPUT_KEYBOARD;
        input.ki.dwExtraInfo=input.ki.time=input.ki.wVk=0;
        for(auto &key:str){
           input.ki.wScan=key;
           input.ki.wVk=0;
           returnNoHitMapValueReplace(std::unordered_map<wchar_t,std::function<void()> >{
               {
                   ENTERWIDE,[&]()->void{
                    input.ki.wScan=VK_RETURN;
                    input.ki.wVk=MAKEWPARAM(VK_RETURN,0);                       
                   }
                }
           },key,[]()->void{})[key]();
            input.ki.dwFlags=KEYEVENTF_UNICODE;
            SendInput(DATACOUNT,&input,sizeof(INPUT));
            input.ki.dwFlags|=KEYEVENTF_KEYUP;
            SendInput(DATACOUNT,&input,sizeof(INPUT));
        }
    }
    template <class T>
    inline T customBool(const bool check,const T trueT,const T falseT)noexcept(false){
        return check?trueT:falseT;
    }
    std::string &linkStr(std::string &str,const std::vector<std::string> array)noexcept(true){
        for(auto &obj:array){
            str+=obj;
        }
        return str;
    }
    template <class... T>
    std::string &linkStr(std::string &str,const T... adds)noexcept(true){
        return linkStr(str,cast::toString(adds...));
    }
    
    inline auto split(std::string str,const std::string cut) noexcept(false){
        std::vector<std::string> data;
        for(auto pos=str.find(cut);pos!=std::string::npos;pos=str.find(cut)){
            data.push_back(str.substr(0,pos));
            str=str.substr(pos+cut.size());
        }
        return data;
    }
    inline auto vecToString( std::vector<std::string> vec, std::string between="")noexcept(true){
        std::string re;
        for(auto &str:vec){
            re+=str+between;
        }
        return re;
    }

    template <class MapType,class KeyType>
    MapType &editKey(MapType &map, KeyType key, KeyType after)noexcept(false){
        auto node=map.extract(key);
        node.key()=after;
        map.insert(std::move(node));
        return map;
    }

    inline auto getWindowStr(const HWND hwnd)noexcept(false){
        const int BUFSIZE=GetWindowTextLength(hwnd)+1;
        std::unique_ptr<TCHAR> text(new TCHAR[BUFSIZE]);
        GetWindowText(hwnd,text.get(),BUFSIZE);
        return std::string(text.get(),text.get()+BUFSIZE-1);
    }
    inline auto getListStr(const HWND list,const int id)noexcept(false){
        const int BUFSIZE=SendMessage(list,LB_GETTEXTLEN,id,0)+1;
        std::unique_ptr<TCHAR> text(new TCHAR[BUFSIZE]);
        SendMessage(list,LB_GETTEXT,id,(LPARAM)text.get());
        return std::string(text.get(),text.get()+BUFSIZE-1);
    }
    enum modeOperator{
        EQUAL,
        ADD,
        SUB
    };
    inline void windowLong(const HWND hwnd,UINT ws,const int mode=modeOperator::EQUAL,const bool Ex=false)noexcept(true){
        UINT style=Ex ? GWL_EXSTYLE:GWL_STYLE;
        std::unordered_map<int,std::function<void()> >{
            {
                modeOperator::ADD,[&]()->void{ws|=GetWindowLong(hwnd,style);}
            },
            {
                modeOperator::SUB,[&]()->void{ws=GetWindowLong(hwnd,style)&~ws;}
            }
            }[mode]();
        SetWindowLong(hwnd,style,ws);
    }
    template <class RT,class... T>
    RT callFunction(const RT (*function)(T...),T... arguments)noexcept(true){
        return function(arguments...);
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
        typename getT::mapped_type &get(getT &map, std::vector<typename getT::key_type> keys){
            noHitMapValueReplace(map,keys[0]);
            return map.at(keys[0]);
        }

        template <class MapT,class... KeyT>
        typename getT::mapped_type &get( MapT &map, KeyT... keys){
            return get(map,std::vector<typename getT::key_type>{keys...});
        }
    };
    inline auto StrToInt(std::string str,int dataCount){
        std::vector<int> list;
        list.resize(dataCount);
        std::smatch m{};
        for(auto &item:list){
            std::regex_search(str,m,std::regex(R"(\d+)"));
            item=cast::toInt(m[0].str());
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
        inline auto write(const std::string str,const bool reset=false) const noexcept(false){
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
        inline  auto changeFile(const std::string name) noexcept(true) {
            setName(name);
            analysis();
            return *this;
        }
        template<class T,class... KeyT>
        inline typename T::mapped_type getData(const KeyT... keys) noexcept(true){
            return  mapManager<T>().get(data,std::vector<std::string>{keys...});
        }
        template <class T,class... KeyT>
        inline auto deleteData(const std::string delKey,const KeyT... keys)  noexcept(true){
            mapManager<T>().get(data,keys...).erase(delKey);
            return *this;
        }
        inline auto editValue(const std::string section,const std::string key,std::string after)noexcept(true){
            noHitMapValueReplace(data,section);
            noHitMapValueReplace(data[section],key);
            data[section][key]=after;
            return *this;
        }
        inline  auto makeString(){
            std::string textData;
            for(auto &[section,line]:data){
                linkStr(textData,SECTIONLEFT,section,SECTIONRIGHT,constGlobalData::ENTER);
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
    class ResizeManager{
        private:
        HWND hwnd;
        double ratioX,ratioY;
        std::vector<std::pair<HWND,RECT> > children;
        public:
        ResizeManager(HWND hwnd,std::vector<HWND> children):hwnd(hwnd){
            RECT rect;
            GetClientRect(hwnd,&rect);
            ratioX=rect.right;
            ratioY=rect.bottom;
            WINDOWPLACEMENT pos;
            pos.length=sizeof(WINDOWPLACEMENT);
            for(auto &hw:children){
                GetWindowPlacement(hw,&pos);
                this->children.push_back({hw,pos.rcNormalPosition});
            }
        }
        ResizeManager(){

        }
        void resize(){
            RECT rect;
            GetClientRect(hwnd,&rect);
            double ratioX=rect.right/this->ratioX;
            double ratioY=rect.bottom/this->ratioY;
            for(auto &[hw,re]:children){//memory leak
                MoveWindow(hw,re.left*ratioX,re.top*ratioY,ratioX*(re.right-re.left),ratioY*(re.bottom-re.top),TRUE);
            }
        }
    };
    inline auto baseStyle(WNDPROC wndproc,LPCSTR name){
        WNDCLASS winc;
        winc.style=CS_VREDRAW|CS_HREDRAW|CS_DBLCLKS;
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
    struct scrollData{
        int start,end,now,nowc;
        bool is_move;
    };
    struct btnData{
        bool flag;
        int msg;
    };
    struct listData{
        std::vector<std::string> list;
        int active;
        int showItem;
        int msg;
        HWND scroll;
    };
    struct menuData{
        std::vector<std::pair<std::string,std::vector<std::pair<std::string,int> > > > obj;
    };
    namespace global{
    POINT now,
          hash;
    int flagMouseWndEvent;
    enum MOUSEWNDEVENT{
        RESIZE_RB,
        RESIZE_LT,
        RESIZE_RT,
        RESIZE_LB,
        RESIZE_R,
        RESIZE_B,
        RESIZE_L,
        RESIZE_T,
        MOVE
    };
    };
    VOID CALLBACK mouseWndEventProc(HWND hwnd , UINT uMsg ,UINT idEvent , DWORD dwTime) {
    if(0<=GetAsyncKeyState(VK_LBUTTON)){
        KillTimer(hwnd,idEvent);
    }
    POINT pos;
    GetCursorPos(&pos);
    std::unordered_map<int,std::function<void()> >{//probable , you should use array or bit set
        {global::MOUSEWNDEVENT::MOVE,[&]()->void{
            MoveWindow(hwnd,pos.x-h::global::hash.x,pos.y-h::global::hash.y,h::global::now.x,h::global::now.y,TRUE);
        }},
        {global::MOUSEWNDEVENT::RESIZE_LT,[&]()->void{
            MoveWindow(hwnd,pos.x,pos.y,h::global::now.x-pos.x,h::global::now.y-pos.y,TRUE);
        }},
        {global::MOUSEWNDEVENT::RESIZE_LB,[&]()->void{
            MoveWindow(hwnd,pos.x,global::now.y,h::global::now.x-pos.x,pos.y-h::global::now.y,TRUE);
        }},
        {global::MOUSEWNDEVENT::RESIZE_L,[&]()->void{
            MoveWindow(hwnd,pos.x,global::now.y,h::global::now.x-pos.x,h::global::hash.y,TRUE);
        }},
        {global::MOUSEWNDEVENT::RESIZE_T,[&]()->void{
            MoveWindow(hwnd,global::now.x,pos.y,h::global::hash.x,h::global::now.y-pos.y,TRUE);
        }},
        {global::MOUSEWNDEVENT::RESIZE_RB,[&]()->void{
            MoveWindow(hwnd,h::global::now.x,h::global::now.y,pos.x-h::global::now.x,pos.y-h::global::now.y,TRUE);
        }},
        {global::MOUSEWNDEVENT::RESIZE_RT,[&]()->void{
            MoveWindow(hwnd,h::global::now.x,pos.y,pos.x-h::global::now.x,h::global::now.y-pos.y,TRUE);
        }},
        {global::MOUSEWNDEVENT::RESIZE_R,[&]()->void{
            MoveWindow(hwnd,h::global::now.x,h::global::now.y,pos.x-h::global::now.x,h::global::hash.y,TRUE);
        }},
        {global::MOUSEWNDEVENT::RESIZE_B,[&]()->void{
            MoveWindow(hwnd,h::global::now.x,h::global::now.y,h::global::hash.x,pos.y-h::global::now.y,TRUE);
        }}

    }[global::flagMouseWndEvent]();
    }
    BOOL CALLBACK addGlobalHwndsChild(HWND hwnd,LPARAM lp){
    h::global::hwnds.push_back(hwnd);
    return TRUE;
    }
    class WndProcWM{
    public:
    virtual  LRESULT CALLBACK Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp)=0;
    };

    class WndProc:public WndProcWM{
        protected:
        std::unordered_map<UINT,std::vector<WndProcWM*> > wms;
        public:
        inline LRESULT CALLBACK Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp)override{
            if(!wms.count(msg))return DefWindowProc(hwnd,msg,wp,lp);
            UINT lresult;
            for(auto wndproc:wms[msg]){
                if(wndproc==nullptr)continue;
                lresult=wndproc->Do(hwnd,msg,wp,lp);
            }
            return lresult;
        };
        inline auto &add(UINT msg,WndProcWM *wndProcWM_PTR){
            wms[msg].push_back(wndProcWM_PTR);
            return *this;
        }
        ~WndProc(){
            wms.clear();
        }
    };
    class wndProcWM_Exit:public WndProcWM{
        public:
        inline LRESULT CALLBACK Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp)override;
    };
    class wndProcWM_Create:public WndProcWM{
        public:
        inline LRESULT CALLBACK Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp)override;
    };
    class wndProcWM_Resize:public WndProcWM{
        public:
        inline LRESULT CALLBACK Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp)override;
    };
    class wndProcWM_PaintOfFrame:public WndProcWM{
    public:
        inline LRESULT CALLBACK Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp)override{
            RECT rect;
            GetClientRect(hwnd,&rect);
            auto hdc=GetDC(hwnd);
            FrameRect(hdc,&rect,h::global::borderBrush.getCreated());
            ReleaseDC(hwnd,hdc);
            return DefWindowProc(hwnd,msg,wp,lp);
        }

    };
    class wndProcWM_RBTNDown:public WndProcWM{
        public:
        LRESULT CALLBACK Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp)override;
    };
    class wndProcWM_LBTNDown:public WndProcWM{
        public: 
        LRESULT CALLBACK Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp)override;
    };
    class wndProcWM_ColorEdit:public WndProcWM{
        public:
        LRESULT CALLBACK Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp)override{
            SetTextColor(HDC(wp),h::global::borderBrush.getBase());
            SetBkColor(HDC(wp),h::global::bkBrush.getBase());
        return (LONG)h::global::bkBrush.getCreated();
        }
    };
    class wndProcCMD_Add:public WndProcWM{
        public:
        LRESULT CALLBACK Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp)override;
    };
    class wndProcCMD_Sub:public WndProcWM{
        public:
        LRESULT CALLBACK Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp)override;
    };
    class wndProcCMD_DBK:public WndProcWM{
        public:
        LRESULT CALLBACK Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp)override;
    };
    class wndProcCMD_Open:public WndProcWM{
        public:
        LRESULT CALLBACK Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp)override;
    };
    class wndProcCMD_Save:public WndProcWM{
        public:
        LRESULT CALLBACK Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp)override;
    };
    class wndProcCMD_Help:public WndProcWM{
        public:
        LRESULT CALLBACK Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp)override{
            if(!std::filesystem::exists(h::constGlobalData::HELPHTML))h::File(h::constGlobalData::HELPHTML).write(
                "<!DOCTYPE html><html><head><meta charset=\"utf-8\"><title>EasyHelp</title></head><body><h1>モードについて</h1><div>ウィンドウ内を右クリック:モード変更</div><div>ウィンドウのタイトルバーを見るとモードが書いてある</div><div>Mode:Inputの場合は本ソフトウェアにテキストを書き込めます</div><div>Mode:Outputの場合は入力したい場所にフォーカスをあて入力します</div><div>※入力については後述</div><h1>本ソフトウェアそれぞれのウィンドウについて</h1><div>タイトルバーの無いウィンドウがテキストボックスになります</div><div>※テキストボックスは枠の色が変わっていることろをドラッグするとサイズ変更と疑似的な移動ができます</div><div>\"wordList\"は自動入力するデータを保存し自動入力に使われます</div><div>\"Add\"はテキストボックスの内容をリストに追加します</div><div>\"Sub\"は選択されているリストのデータを削除します</div><h1>入力について</h1><div>モードを\"Mode:Output\"に変更し入力したい場所にフォーカスを当てリストをダブルクリックしてください</div><h1>ファイルの読み取りとセーブについて</h1><div>本ソフトウェアのタイトルバー付近にある\"FILE (F)\"について説明します</div><div>\"FILE(F)\"の\"FILE OPEN\"でファイル選択ウィンドウが開きます</div><div>※ファイル選択については省略</div><div>開いたファイルの[CELEND]までを一つのデータとしリストに表示します</div><div>その際にメッセージボックスが開いてリストをリセットするか聞かれるので「はい」を選択とリストがリセットされ「いいえ」を選択するとそのまま追加されます</div><div>\"FILE(F)\"の\"FILE SAVE\"で同じようにファイル選択ウィンドウが開きます</div><div>開くと先ほどと同じようなファイルをリセットするかファイルの最後に追加するかを聞かれます</div><div>※選択後の説明は略</div><h1>設定ファイルについて</h1><div>本ソフトウェアと同じフォルダ（ディレクトリ）内にあるsetting(.ini)について説明します</div><div>このファイルは起動時に同じフォルダ内に見つからない場合新しく作成されます</div><div>それぞれのデータがどのように機能するかは多すぎるので書きません</div></body></html>"
            );
            system(std::filesystem::absolute(h::constGlobalData::HELPHTML).string().c_str());
            return DefWindowProc(hwnd,msg,wp,lp);
        }
        
    };
    class wndProcCMD_Setting:public WndProc{
        LRESULT CALLBACK Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp)override;
    };
    class wndProcCMD:public WndProc{
        public:
        inline LRESULT CALLBACK Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp)override{
            if(!wms.count(wp))return DefWindowProc(hwnd,msg,wp,lp);
            LRESULT l;
            for(auto &obj:wms.at(wp)){
                if(obj==nullptr)continue;
                l=obj->Do(hwnd,msg,wp,lp);
            }
            return l;
        }
    };
    class wndProcCMD_c :public wndProcCMD{
        private:
        wndProcCMD_Add add_c;
        wndProcCMD_Sub sub;
        wndProcCMD_Open open;
        wndProcCMD_Save save;
        wndProcCMD_Help help;
        wndProcCMD_DBK dbk;
        wndProcCMD_Setting setting;
        public:
        wndProcCMD_c();
    };
    class mainProc:public WndProc{
        public:
        enum HWNDS{
            title,
            sub,
            edit,
            list
        };
        enum MSG{
            Add=10,
            Sub,
            LIST_DBK,
            MENU_FILE_OPEN,
            MENU_FILE_SAVE,
            MENU_SETTING,
            MENU_HEIP
        };      
        private:
        static HWND hwnds[list+1];
        wndProcWM_Exit exit;
        wndProcWM_Create create;
        wndProcWM_Resize resize;
        wndProcWM_PaintOfFrame paintOfFrame;
        wndProcWM_RBTNDown RBtnDown;
        wndProcWM_LBTNDown LBtnDown;
        wndProcWM_ColorEdit colorEdit;
        wndProcCMD_c cmd;
        static ResizeManager rm;
        static OPENFILENAME ofn;
        static TCHAR path[MAX_PATH];
        public:
        static auto setOwner(HWND hwnd){
            ofn.hwndOwner=hwnd;
        }
        static auto setRm(HWND hwnd,std::vector<HWND> list){
            rm=std::move(ResizeManager(hwnd,list));
        }
        static auto getRm(){
            return rm;
        }
        mainProc(){
            ofn.lStructSize=sizeof(OPENFILENAME);
            ofn.lpstrFilter=TEXT("All files {*.*}\0*.*\0\0");
            ofn.lpstrCustomFilter=0;
            ofn.lpstrFile=path;
            ofn.nMaxFile=MAX_PATH;
            ofn.Flags=OFN_NOVALIDATE;
            add(WM_CREATE,&create);
            add(WM_SIZE,&resize);
            add(WM_DESTROY,&exit);
            add(WM_PAINT,&paintOfFrame);
            add(WM_RBUTTONDOWN,&RBtnDown);
            add(WM_LBUTTONDOWN,&LBtnDown);
            add(WM_CTLCOLOREDIT,&colorEdit);
            add(WM_COMMAND,&cmd);
        }
        static auto setHwnd(HWND hwnd,int n){
            return hwnds[n]=hwnd;
            }
        static auto getHwnd(int n){
            return hwnds[n];
        }
        static auto fileOpen(){
            return GetOpenFileName(&ofn);
        }
        static auto getPath(){
            return path;
        }
    };
    HWND mainProc::hwnds[list+1];
    ResizeManager mainProc::rm;
    OPENFILENAME mainProc::ofn{0};
    TCHAR mainProc::path[MAX_PATH];
    wndProcCMD_c::wndProcCMD_c(){
        add(mainProc::MSG::Add,&add_c);
        add(mainProc::MSG::Sub,&sub);
        add(mainProc::MSG::LIST_DBK,&dbk);
        add(mainProc::MSG::MENU_FILE_OPEN,&open);
        add(mainProc::MSG::MENU_FILE_SAVE,&save);
        add(mainProc::MSG::MENU_HEIP,&help);
        add(mainProc::MSG::MENU_SETTING,&setting);
    }
    LRESULT CALLBACK wndProcWM_Exit::Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp){
            RECT rect;
            GetWindowRect(hwnd,&rect);
            h::INI(h::constGlobalData::SETTING_FILE)
            .editValue(h::constGlobalData::SECTION_STRING,h::constGlobalData::KEY_WORDLIST,h::vecToString(((struct h::listData*)(SendMessage(mainProc::getHwnd(mainProc::HWNDS::list),WM_COMMAND,h::constGlobalData::LIST::GET_OBJ,0)))->list,h::constGlobalData::CELEND))
            .editValue(h::constGlobalData::SECTION_POS,MAINWINDOWNAME,h::vecToString(h::cast::toString(rect.left,rect.top,rect.right-rect.left,rect.bottom-rect.top)," "))
            .save();
            PostQuitMessage(0);
            
        return DefWindowProc(hwnd,msg,wp,lp);
    }
    inline LRESULT CALLBACK wndProcWM_Create::Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp){
            RECT rect;
            if(!std::filesystem::exists(h::constGlobalData::SETTING_FILE)){
                std::ofstream(h::constGlobalData::SETTING_FILE);
            }
            SetLayeredWindowAttributes(hwnd,RGB(0,1,0),h::StrToInt(h::INI(h::constGlobalData::SETTING_FILE).getData<h::INIT::keyT>(h::constGlobalData::SECTION_SHOW,h::constGlobalData::KEY_ALPHA),1)[0],LWA_ALPHA);
            GetClientRect(hwnd,&rect);
            h::global::now.x=(rect.right-rect.left)/50;
            h::global::now.y=(rect.bottom-rect.top)/50;
            rect.left+=h::global::now.x;
            rect.right-=h::global::now.x;
            rect.top+=h::global::now.y;
            rect.bottom-=h::global::now.y;
            mainProc::setHwnd(CreateWindow(TEXT("title"),TEXT(h::constGlobalData::MODE_INPUT),WS_VISIBLE|WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS,rect.left,rect.top,rect.right-rect.left,(rect.bottom-rect.top)/10,hwnd,NULL,LPCREATESTRUCT(lp)->hInstance,NULL),mainProc::HWNDS::title);
            rect.top+=(rect.bottom-rect.top)/10;;
            h::menuData menuData{
                {
                    {"File",
                        {
                            {"Open",mainProc::MSG::MENU_FILE_OPEN},
                            {"Save",mainProc::MSG::MENU_FILE_SAVE}
                        }
                    },
                    {
                        "Setting",
                        {
                            {"Open",mainProc::MSG::MENU_SETTING}
                        }
                    },
                    {"Help",
                        {
                            {"Open",mainProc::MSG::MENU_HEIP}
                        }
                    }
                }
            };
            SendMessage(CreateWindow(TEXT("MENU"),TEXT(""),WS_VISIBLE|WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS,rect.left,rect.top,rect.right-rect.left,(rect.bottom-rect.top)/10,hwnd,NULL,LPCREATESTRUCT(lp)->hInstance,NULL),WM_COMMAND,h::constGlobalData::MENU::SET_MENU,(LPARAM)&menuData);
            rect.top+=(rect.bottom-rect.top)/10;
            h::btnData btnData{false,mainProc::MSG::Add};
            SendMessage(CreateWindow(TEXT(h::constGlobalData::SIMPLEBTN_WINDOW),TEXT("Add"),BS_PUSHBUTTON|WS_VISIBLE|WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|BS_MULTILINE,((rect.right-rect.left)/10)*8,rect.top,rect.right-((rect.right-rect.left)/10)*8,(rect.bottom-rect.top)/2,hwnd,NULL,LPCREATESTRUCT(lp)->hInstance,NULL),WM_COMMAND,h::constGlobalData::BTN::SET_BTN,(LPARAM)&btnData);
            btnData.msg=mainProc::MSG::Sub;
            mainProc::setHwnd(CreateWindow(TEXT(h::constGlobalData::SIMPLEBTN_WINDOW),TEXT("Sub"),BS_PUSHBUTTON|WS_VISIBLE|WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|BS_MULTILINE,((rect.right-rect.left)/10)*8,rect.top+(rect.bottom-rect.top)/2,rect.right-((rect.right-rect.left)/10)*8,(rect.bottom-rect.top)/2,hwnd,NULL,LPCREATESTRUCT(lp)->hInstance,NULL),mainProc::HWNDS::sub);
            SendMessage(mainProc::getHwnd(mainProc::HWNDS::sub),WM_COMMAND,h::constGlobalData::BTN::SET_BTN,(LPARAM)&btnData);
            h::listData listData{h::split(h::INI(h::constGlobalData::SETTING_FILE).getData<h::INIT::keyT>(h::constGlobalData::SECTION_STRING,h::constGlobalData::KEY_WORDLIST),h::constGlobalData::CELEND),0,10,mainProc::MSG::LIST_DBK,NULL};
            mainProc::setHwnd(CreateWindow(TEXT(h::constGlobalData::SIMPLELIST_WINDOW),TEXT(""),BS_PUSHBUTTON|WS_VISIBLE|WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|BS_MULTILINE,rect.left,rect.top,((rect.right-rect.left)/10)*8-rect.left,(rect.bottom-rect.top)/2,hwnd,NULL,LPCREATESTRUCT(lp)->hInstance,NULL),mainProc::HWNDS::list);
            SendMessage(mainProc::getHwnd(mainProc::HWNDS::list),WM_COMMAND,h::constGlobalData::LIST::SET_LIST,(LPARAM)&listData);
            mainProc::setHwnd(CreateWindow(TEXT("EDIT"),TEXT(""),ES_AUTOHSCROLL|ES_AUTOVSCROLL|WS_HSCROLL|WS_VSCROLL|WS_VISIBLE|WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|ES_MULTILINE|ES_WANTRETURN,rect.left,rect.top+(rect.bottom-rect.top)/2,((rect.right-rect.left)/10)*8-rect.left,(rect.bottom-rect.top)/2,hwnd,NULL,LPCREATESTRUCT(lp)->hInstance,NULL),mainProc::HWNDS::edit);
            SendMessage(mainProc::getHwnd(mainProc::HWNDS::edit),WM_SETFONT,(WPARAM)h::global::font.setHeight(20).getCreated(),MAKELPARAM(FALSE,0));
            mainProc::setOwner(hwnd);
            EnumChildWindows(hwnd,addGlobalHwndsChild,0);
            mainProc::setRm(hwnd,h::global::hwnds);
        return DefWindowProc(hwnd,msg,wp,lp);
    }
     inline LRESULT CALLBACK wndProcWM_Resize::Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp){
         mainProc::getRm().resize();
         return DefWindowProc(hwnd,msg,wp,lp);
     }
     inline LRESULT CALLBACK wndProcWM_RBTNDown::Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp){
            h::windowLong(hwnd,WS_EX_NOACTIVATE,h::customBool(h::getWindowStr(mainProc::getHwnd(mainProc::HWNDS::title)).compare(h::constGlobalData::MODE_INPUT),h::modeOperator::SUB,h::modeOperator::ADD),true);
            SetWindowText(mainProc::getHwnd(mainProc::HWNDS::title),h::customBool(h::getWindowStr(mainProc::getHwnd(mainProc::HWNDS::title)).compare(h::constGlobalData::MODE_INPUT),TEXT(h::constGlobalData::MODE_INPUT),TEXT(h::constGlobalData::MODE_OUTPUT)));
           InvalidateRect(mainProc::getHwnd(mainProc::HWNDS::title),NULL,TRUE);
             UpdateWindow(mainProc::getHwnd(mainProc::HWNDS::title));
             return DefWindowProc(hwnd,msg,wp,lp);
     }
     inline LRESULT CALLBACK wndProcWM_LBTNDown::Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp){
        RECT rect,rb;
        GetWindowRect(mainProc::getHwnd(mainProc::HWNDS::list),&rect);
        GetWindowRect(mainProc::getHwnd(mainProc::HWNDS::sub),&rb);
        GetCursorPos(&h::global::now);
        if(h::global::now.x<rect.left&&h::global::now.y<rect.top){//bit flag lt
               GetWindowRect(hwnd,&rect);
               h::global::now.x=rect.right;
               h::global::now.y=rect.bottom;
               global::flagMouseWndEvent=global::MOUSEWNDEVENT::RESIZE_LT;
               SetTimer(hwnd,0,h::constGlobalData::MOUSE_UPDATA_COUNT,mouseWndEventProc);        
               return DefWindowProc(hwnd,msg,wp,lp);
        }
        if(h::global::now.x<rect.left&&rb.bottom<h::global::now.y){//bit flag lb
               GetWindowRect(hwnd,&rect);
               h::global::now.x=rect.right;
               h::global::now.y=rect.top;
               global::flagMouseWndEvent=global::MOUSEWNDEVENT::RESIZE_LB;
               SetTimer(hwnd,0,h::constGlobalData::MOUSE_UPDATA_COUNT,mouseWndEventProc);        
               return DefWindowProc(hwnd,msg,wp,lp);
        }
        if(h::global::now.y<rect.top&&rb.right<h::global::now.x){//bit flag rt
               GetWindowRect(hwnd,&rect);
               h::global::now.x=rect.left;
               h::global::now.y=rect.bottom;
               global::flagMouseWndEvent=global::MOUSEWNDEVENT::RESIZE_RT;
               SetTimer(hwnd,0,h::constGlobalData::MOUSE_UPDATA_COUNT,mouseWndEventProc);        
               return DefWindowProc(hwnd,msg,wp,lp);
        }
        if(h::global::now.x<rect.left){
               GetWindowRect(hwnd,&rect);
               h::global::hash.y=rect.bottom-rect.top;
               h::global::now.x=rect.right;
               h::global::now.y=rect.top;
               global::flagMouseWndEvent=global::MOUSEWNDEVENT::RESIZE_L;
               SetTimer(hwnd,0,h::constGlobalData::MOUSE_UPDATA_COUNT,mouseWndEventProc);        
               return DefWindowProc(hwnd,msg,wp,lp);
        }
        if(h::global::now.y<rect.top){
               GetWindowRect(hwnd,&rect);
               h::global::hash.x=rect.right-rect.left;
               h::global::now.x=rect.left;
               h::global::now.y=rect.bottom;
               global::flagMouseWndEvent=global::MOUSEWNDEVENT::RESIZE_T;
               SetTimer(hwnd,0,h::constGlobalData::MOUSE_UPDATA_COUNT,mouseWndEventProc);        
               return DefWindowProc(hwnd,msg,wp,lp);
        }
        //////////////////////////////////////////////////
        if(rb.right<h::global::now.x&&rb.bottom<h::global::now.y){
            GetWindowRect(hwnd,&rect);
            h::global::now.x=rect.left;
            h::global::now.y=rect.top;
            global::flagMouseWndEvent=global::MOUSEWNDEVENT::RESIZE_RB;
            SetTimer(hwnd,global::MOUSEWNDEVENT::RESIZE_RB,h::constGlobalData::MOUSE_UPDATA_COUNT,mouseWndEventProc);
            return DefWindowProc(hwnd,msg,wp,lp);
        }
        if(rb.right<h::global::now.x){
            GetWindowRect(hwnd,&rect);
            h::global::hash.y=rect.bottom-rect.top;
            h::global::now.x=rect.left;
            h::global::now.y=rect.top;
            global::flagMouseWndEvent=global::MOUSEWNDEVENT::RESIZE_R;
            SetTimer(hwnd,global::MOUSEWNDEVENT::RESIZE_R,h::constGlobalData::MOUSE_UPDATA_COUNT,mouseWndEventProc);
            return DefWindowProc(hwnd,msg,wp,lp);
        }
        if(rb.bottom<h::global::now.y){
            GetWindowRect(hwnd,&rect);
            h::global::hash.x=rect.right-rect.left;
            h::global::now.x=rect.left;
            h::global::now.y=rect.top;
            global::flagMouseWndEvent=global::MOUSEWNDEVENT::RESIZE_B;
            SetTimer(hwnd,global::MOUSEWNDEVENT::RESIZE_B/*flag=b*/,h::constGlobalData::MOUSE_UPDATA_COUNT,mouseWndEventProc);
            return DefWindowProc(hwnd,msg,wp,lp);
        }
        return DefWindowProc(hwnd,msg,wp,lp);
     }
     inline LRESULT CALLBACK wndProcCMD_Add::Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp){
         auto str=h::replaceAll(h::getWindowStr(mainProc::getHwnd(mainProc::HWNDS::edit)),"\r\n","[ENTER]");
         SendMessage(mainProc::getHwnd(mainProc::HWNDS::list),WM_COMMAND,h::constGlobalData::LIST::PUSH,(LPARAM)&str);
         return DefWindowProc(hwnd,msg,wp,lp);
     }
     inline LRESULT CALLBACK wndProcCMD_Sub::Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp){
         SendMessage(mainProc::getHwnd(mainProc::HWNDS::list),WM_COMMAND,h::constGlobalData::LIST::DELETE_ITEM,SendMessage(mainProc::getHwnd(mainProc::HWNDS::list),WM_COMMAND,h::constGlobalData::LIST::GET_SELECT_INDEX,0));
         return DefWindowProc(hwnd,msg,wp,lp);
    }
    inline LRESULT CALLBACK wndProcCMD_DBK::Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp){
        h::pressKeyAll(h::stringToWstring(h::replaceAll(((std::string*)SendMessage(mainProc::getHwnd(mainProc::HWNDS::list),WM_COMMAND,h::constGlobalData::LIST::GET_ITEM,SendMessage(mainProc::getHwnd(mainProc::HWNDS::list),WM_COMMAND,h::constGlobalData::LIST::GET_SELECT_INDEX,0)))->c_str(),R"(\[ENTER\])",h::constGlobalData::ENTER)));
         return DefWindowProc(hwnd,msg,wp,lp);
    } 
    inline LRESULT CALLBACK wndProcCMD_Open::Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp){
        if(!mainProc::fileOpen())return DefWindowProc(hwnd,msg,wp,lp);
        auto l=((struct h::listData*)(SendMessage(mainProc::getHwnd(mainProc::HWNDS::list),WM_COMMAND,h::constGlobalData::LIST::GET_OBJ,0)))->list;
        std::unordered_map<int/*bool use[msgbox==idyes]*/,std::function<void()> >{
            {//true
                IDYES,[&]()->void{
                    l=h::split(h::File(mainProc::getPath()).read().getContent(),h::constGlobalData::CELEND);
                }
            },
            {//false
                IDNO,[&]()->void{
                    auto a=h::split(h::File(mainProc::getPath()).read().getContent(),h::constGlobalData::CELEND);
                    l.insert(l.end(),a.begin(),a.end());
                }
            }
        }[MessageBox(hwnd,TEXT("reset?"),TEXT("Question"),MB_ICONQUESTION|MB_YESNO)]();
            ((struct h::listData*)(SendMessage(mainProc::getHwnd(mainProc::HWNDS::list),WM_COMMAND,h::constGlobalData::LIST::GET_OBJ,0)))->list=l;
            SendMessage(((struct h::listData*)(SendMessage(mainProc::getHwnd(mainProc::HWNDS::list),WM_COMMAND,h::constGlobalData::LIST::GET_OBJ,0)))->scroll,WM_COMMAND,h::constGlobalData::SCROLL::SET_END,l.size());
        InvalidateRect(mainProc::getHwnd(mainProc::HWNDS::list),NULL,TRUE);
        UpdateWindow(mainProc::getHwnd(mainProc::HWNDS::list));
        return DefWindowProc(hwnd,msg,wp,lp);
    }
    
    inline LRESULT CALLBACK wndProcCMD_Save::Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp){
        if(!mainProc::fileOpen())return DefWindowProc(hwnd,msg,wp,lp);
        h::File(mainProc::getPath()).write(h::vecToString(((struct h::listData*)(SendMessage(mainProc::getHwnd(mainProc::HWNDS::list),WM_COMMAND,h::constGlobalData::LIST::GET_OBJ,0)))->list,h::constGlobalData::CELEND),MessageBox(hwnd,TEXT("reset?"),TEXT("Question"),MB_ICONQUESTION|MB_YESNO)==IDYES);
        return DefWindowProc(hwnd,msg,wp,lp);
    }
    inline LRESULT  CALLBACK wndProcCMD_Setting::Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp){
        CreateWindow(TEXT(h::constGlobalData::SETTING_WINDOW),TEXT(h::constGlobalData::SETTING_WINDOW),WS_VISIBLE|WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN|WS_CLIPSIBLINGS,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,NULL,NULL,(HINSTANCE)GetModuleHandle(0),NULL);
        return DefWindowProc(hwnd,msg,wp,lp);
    }
    class btnProcWM_LBTNDown:public WndProcWM{
    inline LRESULT CALLBACK Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp);
};
    class btnProcWM_LBTNUp:public WndProcWM{
    inline LRESULT CALLBACK Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp);
};
    class btnProcWM_Paint:public WndProcWM{
    inline LRESULT  CALLBACK Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp);
};
    class btnProcCmd_Set:public WndProcWM{
    inline LRESULT  CALLBACK Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp)override;
};
    class btnProcCmd:public wndProcCMD{
    private:
    btnProcCmd_Set set;
    public:
    btnProcCmd(){
        add(h::constGlobalData::BTN::SET_BTN,&set);
    }
};
    class btnProc:public WndProc{
    private:
    static std::unordered_map<HWND,btnData> data;
    btnProcWM_LBTNDown lBtnDown;
    btnProcWM_LBTNUp lBtnUp;
    btnProcWM_Paint paint;
    btnProcCmd cmd;
    public:
    static auto &get(HWND hwnd){
        return data[hwnd];
    }
    btnProc(){
        add(WM_LBUTTONDOWN,&lBtnDown);
        add(WM_LBUTTONUP,&lBtnUp);
        add(WM_PAINT,&paint);
        add(WM_COMMAND,&cmd);
    }
};
    std::unordered_map<HWND,btnData> btnProc::data;
    inline LRESULT CALLBACK btnProcWM_LBTNDown::Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp){
    btnProc::get(hwnd).flag=true;
    InvalidateRect(hwnd,NULL,TRUE);
    UpdateWindow(hwnd);
    return DefWindowProc(hwnd,msg,wp,lp);
}
    inline LRESULT CALLBACK btnProcWM_LBTNUp::Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp){
        btnProc::get(hwnd).flag=false;
    InvalidateRect(hwnd,NULL,TRUE);
    UpdateWindow(hwnd);
    if(GetParent(hwnd)==NULL)return DefWindowProc(hwnd,msg,wp,lp);
    SendMessage(GetParent(hwnd),WM_COMMAND,btnProc::get(hwnd).msg,0);
    return DefWindowProc(hwnd,msg,wp,lp);
}
    inline LRESULT CALLBACK btnProcWM_Paint::Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp){
    RECT rect;
    PAINTSTRUCT ps;
    GetClientRect(hwnd,&rect);
    auto hdc=BeginPaint(hwnd,&ps);
    SetTextColor(hdc,h::customBool(btnProc::get(hwnd).flag,h::global::bkBrush.getBase(),h::global::borderBrush.getBase()));
    SetBkColor(hdc,h::customBool(btnProc::get(hwnd).flag,h::global::borderBrush.getBase(),h::global::bkBrush.getBase()));
    SelectObject(hdc,h::customBool(btnProc::get(hwnd).flag,h::global::borderBrush.getCreated(),h::global::bkBrush.getCreated()));
    Rectangle(hdc,0,0,rect.right,rect.bottom);
    SelectObject(hdc,h::global::font.setHeight(rect.bottom/2).getCreated());
    DrawText(hdc,h::getWindowStr(hwnd).c_str(),-1,&rect,DT_CENTER|DT_WORDBREAK|DT_VCENTER);
    FrameRect(hdc,&rect,h::customBool(btnProc::get(hwnd).flag,h::global::bkBrush.getCreated(),h::global::borderBrush.getCreated()));
    EndPaint(hwnd,&ps);
    return DefWindowProc(hwnd,msg,wp,lp);
}
    inline LRESULT  CALLBACK btnProcCmd_Set::Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp){
            btnProc::get(hwnd).flag=((struct h::btnData*)lp)->flag;    
            btnProc::get(hwnd).msg=((struct h::btnData*)lp)->msg;
            return DefWindowProc(hwnd,msg,wp,lp);
}
    class titleProcWM_Create:public WndProcWM{
    public:
    inline LRESULT  CALLBACK Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp)override;
};
    class titleProcWM_LBTNDown:public WndProcWM{
    public:
    inline LRESULT  CALLBACK Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp)override;
};
    class titleProcWM_Paint:public WndProcWM{
    public:
    inline LRESULT  CALLBACK Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp)override{
        RECT rect;
        PAINTSTRUCT ps;
        GetClientRect(hwnd,&rect);
        auto hdc=BeginPaint(hwnd,&ps);
        FrameRect(hdc,&rect,h::global::borderBrush.getCreated());
        SelectObject(hdc,h::global::font.setHeight(rect.bottom).getCreated());
        SetTextColor(hdc,h::global::borderBrush.getBase());
        SetBkColor(hdc,h::global::bkBrush.getBase());
        DrawText(hdc,h::getWindowStr(hwnd).c_str(),-1,&rect,DT_CENTER|DT_WORDBREAK|DT_VCENTER);
        EndPaint(hwnd,&ps);
        return DefWindowProc(hwnd,msg,wp,lp);
    }
};
    class titleProcWM_RBTNDown:public WndProcWM{
    public:
    inline LRESULT  CALLBACK Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp)override{
        if(GetParent(hwnd)==NULL)return DefWindowProc(hwnd,msg,wp,lp);;
        SendMessage(GetParent(hwnd),msg,wp,lp);
        return DefWindowProc(hwnd,msg,wp,lp);
    }
};
    class titleProcCmd_Exit:public WndProcWM{
    public:
    inline LRESULT  CALLBACK Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp)override{
        if(GetParent(hwnd)==NULL)return DefWindowProc(hwnd,msg,wp,lp);
        SendMessage(GetParent(hwnd),WM_CLOSE,wp,lp);
        return DefWindowProc(hwnd,msg,wp,lp);
    }
};
    class titleProcCmd:public wndProcCMD{
    private:
    titleProcCmd_Exit exit;
    public:
    titleProcCmd();
};
    class titleProc:public WndProc{
    public:
    enum constData{
        EXIT=10,
        MOVEID
    };
    private:
    titleProcWM_Create create;
    titleProcWM_LBTNDown lBtnDown;
    titleProcWM_Paint paint;
    titleProcWM_RBTNDown rBtnDown;
    titleProcCmd cmd;
    public:
    titleProc(){
        add(WM_CREATE,&create);
        add(WM_LBUTTONDOWN,&lBtnDown);
        add(WM_PAINT,&paint);
        add(WM_RBUTTONDOWN,&rBtnDown);
        add(WM_COMMAND,&cmd);
    }
};
    titleProcCmd::titleProcCmd(){
        add(titleProc::constData::EXIT,&exit);
    };
    inline LRESULT  CALLBACK titleProcWM_LBTNDown::Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp){
        RECT rect;
        POINT pos;
        if(GetParent(hwnd)==NULL)return DefWindowProc(hwnd,msg,wp,lp);;
        GetWindowRect(GetParent(hwnd),&rect);
        GetCursorPos(&pos);
        h::global::hash.x=pos.x-rect.left;
        h::global::hash.y=pos.y-rect.top;
        h::global::now.x=rect.right-rect.left;
        h::global::now.y=rect.bottom-rect.top;
        global::flagMouseWndEvent=global::MOUSEWNDEVENT::MOVE;
        SetTimer(GetParent(hwnd),titleProc::constData::MOVEID,h::constGlobalData::MOUSE_UPDATA_COUNT,h::mouseWndEventProc);
        return DefWindowProc(hwnd,msg,wp,lp);
    }
    inline LRESULT  CALLBACK titleProcWM_Create::Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp){
        RECT rect;
        GetClientRect(hwnd,&rect);
        h::btnData btnData{0,titleProc::constData::EXIT};
        SendMessage(CreateWindow(TEXT(h::constGlobalData::SIMPLEBTN_WINDOW),TEXT("X"),WS_VISIBLE|WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS,rect.right/10*9,0,rect.right/10,rect.bottom,hwnd,NULL,LPCREATESTRUCT(lp)->hInstance,NULL),WM_COMMAND,h::constGlobalData::BTN::SET_BTN,(LPARAM)&btnData);
        return DefWindowProc(hwnd,msg,wp,lp);
    }
    class menuProcCmd_Set:public WndProcWM{
        public:
        LRESULT  CALLBACK Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp)override;
    };
    class menuProcCmd:public wndProcCMD{
        private:
        menuProcCmd_Set set;
        public:
        menuProcCmd(){
            add(h::constGlobalData::MENU::SET_MENU,&set);
        }
    };
    class menuProcCmd_Reset:public WndProcWM{
        public:
        LRESULT  CALLBACK Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp)override;
    };
    class menuProcCmd_ResetFast:public WndProcWM{
         public:
         LRESULT  CALLBACK Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp)override;
    };
    class menuProc:public WndProc{
        private:
        static std::unordered_map<HWND,menuData> data;
        menuProcCmd_ResetFast fast;
        menuProcCmd cmd;
        menuProcCmd_Reset reset;
        public:
        static auto &get(){
            return data;
        }
        menuProc(){
            add(WM_COMMAND,&fast);
            add(WM_COMMAND,&cmd);
            add(WM_COMMAND,&reset);
        }
    };
    decltype(menuProc::data) menuProc::data;
    inline LRESULT  CALLBACK menuProcCmd_Reset::Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp){
            if(wp==h::constGlobalData::MENU::SET_MENU||wp<menuProc::get()[hwnd].obj.size()||GetParent(hwnd)==NULL)return DefWindowProc(hwnd,msg,wp,lp);
            SendMessage(GetParent(hwnd),WM_COMMAND,wp,0);
            h::global::hwnds.clear();
            EnumChildWindows(hwnd,h::addGlobalHwndsChild,0);
            for(auto hw:h::global::hwnds){
                ShowWindow(hw,SW_SHOW);
            }
            return DefWindowProc(hwnd,msg,wp,lp);
    }
    inline LRESULT  CALLBACK menuProcCmd_ResetFast::Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp){
        if(wp<menuProc::get()[hwnd].obj.size()){
            ShowWindow(FindWindowEx(hwnd,NULL,TEXT(h::constGlobalData::SIMPLEBTN_WINDOW),menuProc::get()[hwnd].obj[wp].first.c_str()),SW_HIDE);
        }
        return 0;
    }
    inline LRESULT  CALLBACK menuProcCmd_Set::Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp){
        RECT rect;
        GetClientRect(hwnd,&rect);
        menuProc::get()[hwnd].obj=((struct h::menuData*)lp)->obj;
        int i=0,oneSize=rect.right/menuProc::get()[hwnd].obj.size();
        rect.right=oneSize;
        for(auto &[section,list]:menuProc::get()[hwnd].obj){
            RECT rectl{
                rect.left,rect.top,rect.right,rect.bottom
            };
            int oneSizel=oneSize/list.size();
            rectl.right=rect.left+oneSizel;
            h::btnData btnData{0,i};
            ++i;
            SendMessage(CreateWindow(TEXT(h::constGlobalData::SIMPLEBTN_WINDOW),section.c_str(),WS_VISIBLE|WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS,rect.left,rect.top,rect.right-rect.left,rect.bottom-rect.top,hwnd,NULL,(HINSTANCE)GetModuleHandle(0),NULL),WM_COMMAND,h::constGlobalData::BTN::SET_BTN,(LPARAM)&btnData);
            rect.left+=oneSize;
            rect.right+=oneSize;
            for(auto &[name,msg]:list){
                btnData.msg=msg;
                SendMessage(CreateWindow(TEXT(h::constGlobalData::SIMPLEBTN_WINDOW),name.c_str(),WS_VISIBLE|WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS,rectl.left,rectl.top,rectl.right-rectl.left,rectl.bottom-rectl.top,hwnd,NULL,(HINSTANCE)GetModuleHandle(0),NULL),WM_COMMAND,h::constGlobalData::BTN::SET_BTN,(LPARAM)&btnData);    
                rectl.left+=oneSizel;
                rectl.right+=oneSizel;
            }           
        }
        return 0;
    }
    class listProcWM_LBtnDown:public WndProcWM{
        LRESULT  CALLBACK Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp)override;
    };
    class listProcWM_DBC:public WndProcWM{
        LRESULT  CALLBACK Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp)override;
    };
    class listProcWM_Paint:public WndProcWM{
        public:
        LRESULT  CALLBACK Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp)override;
    };
    class listProcCmd_Push:public WndProcWM{
        public:
        LRESULT  CALLBACK Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp)override;
    };
    
    class listProcCmd_Set:public WndProcWM{
        public:
        LRESULT  CALLBACK Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp)override;
    };
    class listProcCmd_GetSelectIndex:public WndProcWM{
        public:
        LRESULT  CALLBACK Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp)override;
    };
    class listProcCmd_GetItem:public WndProcWM{
        public:
        LRESULT  CALLBACK Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp)override;
    };
    class listProcCmd_GetObj:public WndProcWM{
        public:
        LRESULT  CALLBACK Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp)override;
    };
    class listProcCmd_Delete:public WndProcWM{
        public:
        LRESULT  CALLBACK Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp)override;
    };
    class listProcCmd_Change:public WndProcWM{
        public:
        LRESULT  CALLBACK Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp)override{
            InvalidateRect(hwnd,NULL,TRUE);
            UpdateWindow(hwnd);
            return DefWindowProc(hwnd,msg,wp,lp);
        }
    };
    class listProcCmd:public wndProcCMD{
        private:
        listProcCmd_Set set;
        listProcCmd_Push push;
        listProcCmd_GetSelectIndex index;
        listProcCmd_GetItem item;
        listProcCmd_GetObj obj;
        listProcCmd_Delete deleteItem;
        listProcCmd_Change change;
        public:
        listProcCmd(){
            add(h::constGlobalData::LIST::SET_LIST,&set);
            add(h::constGlobalData::LIST::PUSH,&push);
            add(h::constGlobalData::LIST::GET_SELECT_INDEX,&index);
            add(h::constGlobalData::LIST::GET_ITEM,&item);
            add(h::constGlobalData::LIST::GET_OBJ,&obj);
            add(h::constGlobalData::LIST::DELETE_ITEM,&deleteItem);
            add(h::constGlobalData::SCROLL::CHANGE,&change);
        }
    };
    class listProc:public WndProc{
        private:
        static std::unordered_map<HWND,listData> data;
        static std::string str;
        listProcWM_DBC dbc;
        listProcWM_LBtnDown lBtnDown;
        listProcWM_Paint paint;
        listProcCmd cmd;
        public:
        static auto &get(){
            return data;
        }
        static auto &getStr(){
            return str;
        }
        listProc(){
            add(WM_LBUTTONDBLCLK,&dbc);
            add(WM_LBUTTONDOWN,&lBtnDown);
            add(WM_PAINT,&paint);
            add(WM_COMMAND,&cmd);
        }
    };
    decltype(listProc::data) listProc::data;
    std::string listProc::str;
    LRESULT  CALLBACK listProcWM_LBtnDown::Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp){
        RECT rect;
        GetClientRect(hwnd,&rect);
        listProc::get()[hwnd].active=(MAKEPOINTS(lp).y/(static_cast<double>(rect.bottom)/listProc::get()[hwnd].showItem))+SendMessage(listProc::get()[hwnd].scroll,WM_COMMAND,h::constGlobalData::SCROLL::GET_SCROLL,0)-1;
        InvalidateRect(hwnd,NULL,TRUE);
        UpdateWindow(hwnd);
        return DefWindowProc(hwnd,msg,wp,lp);
    }
    LRESULT  CALLBACK listProcWM_DBC::Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp){
        if(GetParent(hwnd)==NULL)return DefWindowProc(hwnd,msg,wp,lp);
        SendMessage(GetParent(hwnd),WM_COMMAND,listProc::get()[hwnd].msg,0);
        return DefWindowProc(hwnd,msg,wp,lp);
    }   
    LRESULT  CALLBACK listProcWM_Paint::Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp){
        RECT rect;
        PAINTSTRUCT ps;
        GetClientRect(hwnd,&rect);
        auto hdc=BeginPaint(hwnd,&ps);
        SetTextColor(hdc,h::global::borderBrush.getBase());
        SetBkColor(hdc,h::global::bkBrush.getBase());
        int oneSize=static_cast<double>(rect.bottom)/listProc::get()[hwnd].showItem;
        SelectObject(hdc,h::global::font.setHeight(oneSize).getCreated());
        for(int hash=SendMessage(listProc::get()[hwnd].scroll,WM_COMMAND,h::constGlobalData::SCROLL::GET_SCROLL,0),i=1;hash<listProc::get()[hwnd].list.size()&&i<listProc::get()[hwnd].showItem;++hash,++i){
            if(hash==listProc::get()[hwnd].active){
                SetTextColor(hdc,h::global::bkBrush.getBase());
                SetBkColor(hdc,h::global::borderBrush.getBase());
                TextOut(hdc,0,oneSize*i,listProc::get()[hwnd].list[hash].c_str(),listProc::get()[hwnd].list[hash].size());
                SetTextColor(hdc,h::global::borderBrush.getBase());
                SetBkColor(hdc,h::global::bkBrush.getBase());
                continue;
            }
            TextOut(hdc,0,oneSize*i,listProc::get()[hwnd].list[hash].c_str(),listProc::get()[hwnd].list[hash].size());
        }
        for(int i=0,max=oneSize;i<max;++i){
            RECT item{
                0,
                static_cast<int>((oneSize)*i),
                rect.right,
                static_cast<int>((oneSize)*(i+1))
            };
            FrameRect(hdc,&item,h::global::borderBrush.getCreated());
        }
        EndPaint(hwnd,&ps);
        return DefWindowProc(hwnd,msg,wp,lp);
    }
    LRESULT  CALLBACK listProcCmd_Set::Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp){
        RECT rect;
        listProc::get()[hwnd].active=((struct h::listData*)lp)->active;
        listProc::get()[hwnd].list=((struct h::listData*)lp)->list;
        listProc::get()[hwnd].showItem=((struct h::listData*)lp)->showItem;
        listProc::get()[hwnd].msg=((struct h::listData*)lp)->msg;
        GetClientRect(hwnd,&rect);
        if(listProc::get()[hwnd].scroll==NULL)
        listProc::get()[hwnd].scroll=CreateWindowEx(WS_EX_TOPMOST,TEXT(h::constGlobalData::SCROLL_WINDOW),TEXT("Height"),WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS,0,0,rect.right,static_cast<double>(rect.bottom)/listProc::get()[hwnd].showItem,hwnd,NULL,(HINSTANCE)GetModuleHandle(0),NULL);
        h::scrollData scrollData{0,static_cast<int>(listProc::get()[hwnd].list.size()),0,0,false};
        SendMessage(listProc::get()[hwnd].scroll,WM_COMMAND,h::constGlobalData::SCROLL::SET,(LPARAM)&scrollData);
        return DefWindowProc(hwnd,msg,wp,lp);
    }
    LRESULT  CALLBACK listProcCmd_Push::Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp){
            listProc::get()[hwnd].list.push_back(((std::string*)lp)->c_str());
            SendMessage(listProc::get()[hwnd].scroll,WM_COMMAND,h::constGlobalData::SCROLL::SET_END,listProc::get()[hwnd].list.size());
            InvalidateRect(hwnd,NULL,TRUE);
            UpdateWindow(hwnd);
            return DefWindowProc(hwnd,msg,wp,lp);
    }
    LRESULT  CALLBACK listProcCmd_GetSelectIndex::Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp){
        if(0>listProc::get()[hwnd].active||listProc::get()[hwnd].list.size()<=listProc::get()[hwnd].active){
            return (LONG)0;
        }
        return (LONG)listProc::get()[hwnd].active;
    }
    LRESULT  CALLBACK listProcCmd_GetItem::Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp){
        if(0>listProc::get()[hwnd].active||listProc::get()[hwnd].list.size()<=listProc::get()[hwnd].active){
            return (LONG)&listProc::getStr();
        }
        return (LONG)&listProc::get()[hwnd].list[lp];
    }
    LRESULT  CALLBACK listProcCmd_GetObj::Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp){
        return (LONG)&listProc::get()[hwnd];
    }
    LRESULT  CALLBACK listProcCmd_Delete::Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp){
        if(0>lp||listProc::get()[hwnd].list.size()<=lp){
            return DefWindowProc(hwnd,msg,wp,lp);;
        }
        listProc::get()[hwnd].list.erase(listProc::get()[hwnd].list.begin()+lp);
        SendMessage(listProc::get()[hwnd].scroll,WM_COMMAND,h::constGlobalData::SCROLL::SET_END,listProc::get()[hwnd].list.size());
        InvalidateRect(hwnd,NULL,TRUE);
        UpdateWindow(hwnd);        
        return DefWindowProc(hwnd,msg,wp,lp);
    }

};
int CALLBACK EnumFontFamProc(LOGFONT *lf,TEXTMETRIC * tm,DWORD fontType,LPARAM lp){
    if(h::global::vecStr==nullptr)return 0;
    h::global::vecStr->push_back(lf->lfFaceName);
    return 1;
}
namespace h{
    inline void setFontList(decltype(h::global::vecStr) vecStr){
        h::global::vecStr=vecStr;
        auto before=vecStr;
        LOGFONT lf;
        lf.lfCharSet=DEFAULT_CHARSET;
        lf.lfFaceName[0]=lf.lfPitchAndFamily=0;
        HDC hdc;
        hdc=GetDC(0);
        EnumFontFamiliesEx(hdc,&lf,(FONTENUMPROC)EnumFontFamProc,(LONG_PTR)&lf,0);
        ReleaseDC(0,hdc);
        h::global::vecStr=before;
    }
    class settingProc_Create:public WndProcWM{
        public:
        LRESULT CALLBACK Do(HWND hwnd,UINT msg,WPARAM wp ,LPARAM lp)override;
    };
    class settingProc_Destroy:public WndProcWM{
        public:
        LRESULT CALLBACK Do(HWND hwnd,UINT msg,WPARAM wp ,LPARAM lp)override;
    };
    class settingProc_Resize:public WndProcWM{
        public:
        LRESULT CALLBACK Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp)override;
    };
    class settingProc_Paint:public WndProcWM{
        public:
        LRESULT CALLBACK Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp)override;
    };
    
    class settingProcCMD_Change:public WndProcWM{
        public:
        LRESULT CALLBACK Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp)override;
    };
    class settingProcCMD:public wndProcCMD{
        private:
        settingProcCMD_Change change;
        public:
        settingProcCMD(){
            add(h::constGlobalData::SCROLL::CHANGE,&change);
        }
    };
    class settingProc:public WndProc{
        public:
        enum HWNDS{
            alpha,r,g,b,fontList
        };
        private:
        static HWND hwnds[fontList+1];
        static ResizeManager rm;
        settingProc_Destroy destroy;
        settingProc_Create create;
        settingProc_Paint paint;
        settingProc_Resize resize;
        settingProcCMD cmd;
        public:
        static auto setHwnd(HWND hwnd,int n){
            return hwnds[n]=hwnd;
            }
        static auto getHwnd(int n){
            return hwnds[n];
        }
        static auto setRm(HWND hwnd,std::vector<HWND> list){
            rm=std::move(ResizeManager(hwnd,list));
        }
        static auto getRm(){
            return rm;
        }
        settingProc(){
            add(WM_DESTROY,&destroy);
            add(WM_CREATE,&create);
            add(WM_PAINT,&paint);
            add(WM_SIZE,&resize);
            add(WM_COMMAND,&cmd);
        }
    };
    HWND settingProc::hwnds[settingProc::HWNDS::fontList+1];
    ResizeManager settingProc::rm;
    LRESULT CALLBACK settingProc_Paint::Do(HWND hwnd,UINT msg,WPARAM wp ,LPARAM lp){
        PAINTSTRUCT ps;
        RECT rect;
        GetClientRect(hwnd,&rect);
        auto hdc=BeginPaint(hwnd,&ps);
        // global::borderBrush.reset(RGB(SendMessage(settingProc::getHwnd(settingProc::HWNDS::r),WM_COMMAND,h::constGlobalData::SCROLL::GET_SCROLL,0),SendMessage(settingProc::getHwnd(settingProc::HWNDS::g),WM_COMMAND,h::constGlobalData::SCROLL::GET_SCROLL,0),SendMessage(settingProc::getHwnd(settingProc::HWNDS::b),WM_COMMAND,h::constGlobalData::SCROLL::GET_SCROLL,0)));
        // FrameRect(hdc,&rect,global::borderBrush.getCreated());
        FrameRect(hdc,&rect,colorManager(RGB(SendMessage(settingProc::getHwnd(settingProc::HWNDS::r),WM_COMMAND,h::constGlobalData::SCROLL::GET_SCROLL,0),SendMessage(settingProc::getHwnd(settingProc::HWNDS::g),WM_COMMAND,h::constGlobalData::SCROLL::GET_SCROLL,0),SendMessage(settingProc::getHwnd(settingProc::HWNDS::b),WM_COMMAND,h::constGlobalData::SCROLL::GET_SCROLL,0))).getCreated());
        EndPaint(hwnd,&ps);
        return DefWindowProc(hwnd,msg,wp,lp);
    }
    LRESULT CALLBACK settingProc_Create::Do(HWND hwnd,UINT msg,WPARAM wp ,LPARAM lp){
        RECT rect;
        constexpr int MSG1=10;
        GetClientRect(hwnd,&rect);
        settingProc::setHwnd(CreateWindowEx(WS_EX_TOPMOST,TEXT(h::constGlobalData::SCROLL_WINDOW),TEXT("Alpha"),WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS,0,0,rect.right/2,rect.bottom/5,hwnd,NULL,LPCREATESTRUCT(lp)->hInstance,NULL),settingProc::HWNDS::alpha);
        settingProc::setHwnd(CreateWindowEx(WS_EX_TOPMOST,TEXT(h::constGlobalData::SCROLL_WINDOW),TEXT("r"),WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS,0,rect.bottom/5,rect.right/2,rect.bottom/5,hwnd,NULL,LPCREATESTRUCT(lp)->hInstance,NULL),settingProc::HWNDS::r);
        settingProc::setHwnd(CreateWindowEx(WS_EX_TOPMOST,TEXT(h::constGlobalData::SCROLL_WINDOW),TEXT("g"),WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS,0,(rect.bottom/5)*2,rect.right/2,rect.bottom/5,hwnd,NULL,LPCREATESTRUCT(lp)->hInstance,NULL),settingProc::HWNDS::g);
        settingProc::setHwnd(CreateWindowEx(WS_EX_TOPMOST,TEXT(h::constGlobalData::SCROLL_WINDOW),TEXT("b"),WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS,0,(rect.bottom/5)*3,rect.right/2,rect.bottom/5,hwnd,NULL,LPCREATESTRUCT(lp)->hInstance,NULL),settingProc::HWNDS::b);
        settingProc::setHwnd(CreateWindow(TEXT(h::constGlobalData::SIMPLELIST_WINDOW),TEXT(""),WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS,rect.right/2,0,rect.right/2,rect.bottom/5*4,hwnd,NULL,LPCREATESTRUCT(lp)->hInstance,NULL),settingProc::HWNDS::fontList);
        h::scrollData data{0,255,0,0,false};
        SendMessage(settingProc::getHwnd(settingProc::HWNDS::alpha),WM_COMMAND,h::constGlobalData::SCROLL::SET,(LPARAM)&data);
        SendMessage(settingProc::getHwnd(settingProc::HWNDS::r),WM_COMMAND,h::constGlobalData::SCROLL::SET,(LPARAM)&data);
        SendMessage(settingProc::getHwnd(settingProc::HWNDS::g),WM_COMMAND,h::constGlobalData::SCROLL::SET,(LPARAM)&data);
        SendMessage(settingProc::getHwnd(settingProc::HWNDS::b),WM_COMMAND,h::constGlobalData::SCROLL::SET,(LPARAM)&data);
        h::listData listData{{},0,MSG1,0,NULL};
        h::setFontList(&listData.list);
        SendMessage(settingProc::getHwnd(settingProc::HWNDS::fontList),WM_COMMAND,h::constGlobalData::LIST::SET_LIST,LPARAM(&listData));           
        h::global::hwnds.clear();
        EnumChildWindows(hwnd,h::addGlobalHwndsChild,0);
        settingProc::setRm(hwnd,h::global::hwnds);
        return DefWindowProc(hwnd,msg,wp,lp);
    }
    LRESULT CALLBACK settingProc_Destroy::Do(HWND hwnd,UINT msg,WPARAM wp ,LPARAM lp){
        h::INI(h::constGlobalData::SETTING_FILE)
        .editValue(h::constGlobalData::SECTION_SHOW,h::constGlobalData::KEY_ALPHA,h::cast::toString((int)SendMessage(settingProc::getHwnd(settingProc::HWNDS::alpha),WM_COMMAND,h::constGlobalData::SCROLL::GET_SCROLL,0)))
        .editValue(h::constGlobalData::SECTION_SHOW,h::constGlobalData::KEY_BORDER_COLOR,h::vecToString(h::cast::toString(SendMessage(settingProc::getHwnd(settingProc::HWNDS::r),WM_COMMAND,h::constGlobalData::SCROLL::GET_SCROLL,0),SendMessage(settingProc::getHwnd(settingProc::HWNDS::g),WM_COMMAND,h::constGlobalData::SCROLL::GET_SCROLL,0),SendMessage(settingProc::getHwnd(settingProc::HWNDS::b),WM_COMMAND,h::constGlobalData::SCROLL::GET_SCROLL,0))," "))
        .editValue(h::constGlobalData::SECTION_SHOW,h::constGlobalData::KEY_FONT,((std::string*)(SendMessage(settingProc::getHwnd(settingProc::HWNDS::fontList),WM_COMMAND,h::constGlobalData::LIST::GET_ITEM,SendMessage(settingProc::getHwnd(settingProc::HWNDS::fontList),WM_COMMAND,h::constGlobalData::LIST::GET_SELECT_INDEX,0))))->c_str())
        .save();
        return DefWindowProc(hwnd,msg,wp,lp);
    }
    inline LRESULT CALLBACK settingProc_Resize::Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp){
         settingProc::getRm().resize();
         return DefWindowProc(hwnd,msg,wp,lp);
    }
    inline LRESULT CALLBACK settingProcCMD_Change::Do(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp){
        if(HWND(lp)!=settingProc::getHwnd(settingProc::HWNDS::alpha)){
            InvalidateRect(hwnd,NULL,TRUE);
            UpdateWindow(hwnd);
        }
        return DefWindowProc(hwnd,msg,wp,lp);
    }
    class scrollProcWM_Destroy:public WndProcWM{
        public:
        LRESULT CALLBACK Do(HWND hwnd,UINT msg,WPARAM wp ,LPARAM lp)override;
    };
    class scrollProcWM_Mouse:public WndProcWM{
        public:
        LRESULT CALLBACK Do(HWND hwnd,UINT msg,WPARAM wp ,LPARAM lp)override;
    };
    class scrollProcWM_Paint:public WndProcWM{
        public:
        LRESULT CALLBACK Do(HWND hwnd,UINT msg,WPARAM wp ,LPARAM lp)override;
    };
    class scrollProcCmd_Get:public WndProcWM{
        public:
        LRESULT CALLBACK Do(HWND hwnd,UINT msg,WPARAM wp ,LPARAM lp)override;
    };
    class scrollProcCmd_Set:public WndProcWM{
        public:
        LRESULT CALLBACK Do(HWND hwnd,UINT msg,WPARAM wp ,LPARAM lp)override;
    };
    class scrollProcCmd_SetEnd:public WndProcWM{
        public:
        LRESULT CALLBACK Do(HWND hwnd,UINT msg,WPARAM wp ,LPARAM lp)override;
    };
    class scrollProcCmd:public wndProcCMD{
        private:
        scrollProcCmd_Get get;
        scrollProcCmd_Set set;
        scrollProcCmd_SetEnd end;
        public:
        scrollProcCmd(){
            add(h::constGlobalData::SCROLL::GET_SCROLL,&get);
            add(h::constGlobalData::SCROLL::SET,&set);
            add(h::constGlobalData::SCROLL::SET_END,&end);
        }

    };
    class scrollProc:public WndProc{
        private:
        static std::unordered_map<HWND,scrollData> data;
        scrollProcWM_Destroy destroy;
        scrollProcWM_Mouse mouse;
        scrollProcWM_Paint paint;
        scrollProcCmd cmd;
        public:
        scrollProc(){
            add(WM_LBUTTONDOWN,&mouse);
            add(WM_MOUSEMOVE,&mouse);
            add(WM_DESTROY,&destroy);
            add(WM_PAINT,&paint);
            add(WM_COMMAND,&cmd);
        }
        static auto &getData(){
            return data;
        }
    };
    std::unordered_map<HWND,scrollData> scrollProc::data;//decltype
    LRESULT CALLBACK scrollProcWM_Destroy::Do(HWND hwnd,UINT msg,WPARAM wp ,LPARAM lp){
        scrollProc::getData().clear();
        return DefWindowProc(hwnd,msg,wp,lp);

    }
    LRESULT CALLBACK scrollProcWM_Mouse::Do(HWND hwnd,UINT msg,WPARAM wp ,LPARAM lp){
        scrollProc::getData()[hwnd].is_move=0>GetAsyncKeyState(VK_LBUTTON);
        if(!scrollProc::getData()[hwnd].is_move){
            return DefWindowProc(hwnd,msg,wp,lp);
        }
        scrollProc::getData()[hwnd].now=MAKEPOINTS(lp).x;
        InvalidateRect(hwnd,NULL,TRUE);
        UpdateWindow(hwnd);
        return DefWindowProc(hwnd,msg,wp,lp);

    }
    LRESULT CALLBACK scrollProcWM_Paint::Do(HWND hwnd,UINT msg,WPARAM wp ,LPARAM lp){
        RECT rect;
        PAINTSTRUCT ps;
        GetClientRect(hwnd,&rect);
        auto hdc=BeginPaint(hwnd,&ps);
        SelectObject(hdc,h::global::font.setHeight(rect.bottom).getCreated());
        SelectObject(hdc,h::global::borderBrush.getCreated());
        SetTextColor(hdc,h::global::borderBrush.getBase());
        SetBkColor(hdc,h::global::bkBrush.getBase());
        DrawText(hdc,(h::getWindowStr(hwnd)+h::cast::toString(scrollProc::getData()[hwnd].nowc)).c_str(),-1,&rect,DT_CENTER|DT_WORDBREAK|DT_VCENTER);
        Rectangle(hdc,scrollProc::getData()[hwnd].now,0,scrollProc::getData()[hwnd].now+10,rect.bottom);
        FrameRect(hdc,&rect,global::borderBrush.getCreated());
        int nowc=scrollProc::getData()[hwnd].now/(static_cast<double>(rect.right)/scrollProc::getData()[hwnd].end);
        if(scrollProc::getData()[hwnd].nowc!=nowc&&GetParent(hwnd)!=NULL){
        SendMessage(GetParent(hwnd),WM_COMMAND,h::constGlobalData::SCROLL::CHANGE,LPARAM(hwnd));
        }
        scrollProc::getData()[hwnd].nowc=nowc;
        EndPaint(hwnd,&ps);
        return DefWindowProc(hwnd,msg,wp,lp);
    }
    LRESULT CALLBACK scrollProcCmd_Get::Do(HWND hwnd,UINT msg,WPARAM wp ,LPARAM lp){
        return scrollProc::getData()[hwnd].nowc;
    }
    LRESULT CALLBACK scrollProcCmd_Set::Do(HWND hwnd,UINT msg,WPARAM wp ,LPARAM lp){
        scrollProc::getData()[hwnd].start=((struct h::scrollData*)lp)->start;
        scrollProc::getData()[hwnd].end=((struct h::scrollData*)lp)->end;
        scrollProc::getData()[hwnd].now=((struct h::scrollData*)lp)->now;
        scrollProc::getData()[hwnd].is_move=((struct h::scrollData*)lp)->is_move;
        scrollProc::getData()[hwnd].nowc=((struct h::scrollData*)lp)->nowc;
        return DefWindowProc(hwnd,msg,wp,lp);
    }
    LRESULT CALLBACK scrollProcCmd_SetEnd::Do(HWND hwnd,UINT msg,WPARAM wp ,LPARAM lp){
        scrollProc::getData()[hwnd].end=lp;
        return DefWindowProc(hwnd,msg,wp,lp);
    }
};
LRESULT CALLBACK scrollProc(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp){
    return h::scrollProc().Do(hwnd,msg,wp,lp);
}
LRESULT CALLBACK settingProc(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp){
    return h::settingProc().Do(hwnd,msg,wp,lp);
}
LRESULT CALLBACK listProc(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp){
    return h::listProc().Do(hwnd,msg,wp,lp);
}
LRESULT CALLBACK btnProc(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp){
    return h::btnProc().Do(hwnd,msg,wp,lp);
}
LRESULT CALLBACK titleProc(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp){
    return h::titleProc().Do(hwnd,msg,wp,lp);
}
LRESULT CALLBACK menuProc(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp){
    return h::menuProc().Do(hwnd,msg,wp,lp);
}
LRESULT CALLBACK WndProc(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp){
    return h::mainProc().Do(hwnd,msg,wp,lp);
}
int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,PSTR lpCmdLine,int nCmdShow){
    constexpr auto MAIN_WINDOW_CLASS="MAIN";
    enum POS{
        X,Y,WIDTH,HEIGHT
    };
    enum RGB{
        R,G,B
    };
    MSG msg;
    h::baseStyle(WndProc,MAIN_WINDOW_CLASS);
    h::baseStyle(scrollProc,h::constGlobalData::SCROLL_WINDOW);
    h::baseStyle(settingProc,h::constGlobalData::SETTING_WINDOW);
    h::baseStyle(btnProc,h::constGlobalData::SIMPLEBTN_WINDOW);
    h::baseStyle(listProc,h::constGlobalData::SIMPLELIST_WINDOW);
    h::baseStyle(titleProc,"title");
    h::baseStyle(menuProc,"MENU");
    auto re=h::StrToInt(h::INI(h::constGlobalData::SETTING_FILE).getData<h::INIT::keyT>(h::constGlobalData::SECTION_SHOW,h::constGlobalData::KEY_BORDER_COLOR),3);
    h::global::borderBrush.reset(RGB(re[RGB::R],re[RGB::G],re[RGB::B]));
    re=h::StrToInt(h::INI(h::constGlobalData::SETTING_FILE).getData<h::INIT::keyT>(h::constGlobalData::SECTION_SHOW,h::constGlobalData::KEY_BK_COLOR),3);
    h::global::bkBrush.reset(RGB(re[RGB::R],re[RGB::G],re[RGB::B]));
    h::global::font.reset(h::INI(h::constGlobalData::SETTING_FILE).getData<h::INIT::keyT>(h::constGlobalData::SECTION_SHOW,h::constGlobalData::KEY_FONT));
    re=h::StrToInt(h::INI(h::constGlobalData::SETTING_FILE).getData<h::INIT::keyT>(h::constGlobalData::SECTION_POS,MAINWINDOWNAME),4);
    SendMessage(CreateWindowEx(WS_EX_TOPMOST|WS_EX_LAYERED,TEXT(MAIN_WINDOW_CLASS),TEXT(h::constGlobalData::MODE_INPUT),WS_CLIPCHILDREN|WS_VISIBLE|WS_POPUP,re[POS::X],re[POS::Y],re[POS::WIDTH],re[POS::HEIGHT],NULL,NULL,hInstance,NULL),WM_SETICON,ICON_BIG,(LPARAM)LoadIcon(hInstance,h::constGlobalData::ICON_NAME));
    
    while(GetMessage(&msg,NULL,0,0)){
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return msg.wParam;
}