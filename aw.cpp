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
#include <cmath>
#include "rc.h"
namespace h{
        template <class T>
        struct RGB{
            static constexpr int R=0,G=R+1,B=G+1;
            T rgb[3];
        };  
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
                           SETTING_WINDOW="SETTING";
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
    template <class T>
    inline auto absSub(const T left,const T right)noexcept(true){
        return std::abs(left)-std::abs(right);
    }
    template <class T>
    inline auto fitNum(const T num,const T left,const T right)noexcept(true){
        return num-absSub(left,right);
    }
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
            if(key==ENTERWIDE){//key,tostring h::constglobaldata::enter
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
    
    template <class T>
    inline T clip(const T num,const T min,const T max)noexcept(true){
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
    template <class T>
    bool beMapItem(T &map,typename T::key_type key){
        return map.count(key);
    }
    template <class T>
    inline void noHitMapValueReplace(T &map, typename T::key_type key, typename T::mapped_type replace=typename T::mapped_type()){
        if(!beMapItem(map,key))map.emplace(key,replace);
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
            for(auto &[hw,re]:children){
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
    // struct btnListData{
    //     std::vector< std::pair<std::string,std::vector<std::pair<std::string,int>> > > obj;
    // };
    struct menuData{
        std::vector<std::pair<std::string,std::vector<std::pair<std::string,int> > > > obj;
    };
};
BOOL CALLBACK addGlobalHwndsChild(HWND hwnd,LPARAM lp){
    h::global::hwnds.push_back(hwnd);
    return TRUE;
}
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
};
LRESULT CALLBACK scrollProc(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp){
    PAINTSTRUCT ps;
    HDC hdc;
    RECT rect;
    HBRUSH defb;
    static std::unordered_map<HWND,h::scrollData> data;
    switch(msg){
        case WM_DESTROY:
        data.clear();
        break;
        case WM_CREATE:
        data[hwnd];
        break;
        case WM_LBUTTONDOWN:
        case WM_MOUSEMOVE:
        data[hwnd].is_move=0>GetAsyncKeyState(VK_LBUTTON);
        if(!data[hwnd].is_move){
            break;
        }
        data[hwnd].now=MAKEPOINTS(lp).x;
        InvalidateRect(hwnd,NULL,TRUE);
        UpdateWindow(hwnd);
        break;
        case WM_PAINT:
        GetClientRect(hwnd,&rect);
        hdc=BeginPaint(hwnd,&ps);
        SelectObject(hdc,h::global::font.setHeight(rect.bottom).getCreated());
        defb=(HBRUSH)SelectObject(hdc,h::global::borderBrush.getCreated());
        SetTextColor(hdc,h::global::borderBrush.getBase());
        SetBkColor(hdc,h::global::bkBrush.getBase());
        DrawText(hdc,(h::getWindowStr(hwnd)+h::cast::toString(data[hwnd].nowc)).c_str(),-1,&rect,DT_CENTER|DT_WORDBREAK|DT_VCENTER);
        Rectangle(hdc,data[hwnd].now,0,data[hwnd].now+10,rect.bottom);
        FrameRect(hdc,&rect,h::global::borderBrush.getCreated());
        {
            int nowc=data[hwnd].now/(static_cast<double>(rect.right)/data[hwnd].end);
            if(data[hwnd].nowc!=nowc&&GetParent(hwnd)!=NULL){
            SendMessage(GetParent(hwnd),WM_COMMAND,h::constGlobalData::SCROLL::CHANGE,LPARAM(hwnd));
            }
            data[hwnd].nowc=nowc;

        }
        SelectObject(hdc,defb);
        EndPaint(hwnd,&ps);
        break;
        case WM_COMMAND:
            switch(wp){
                case h::constGlobalData::SCROLL::GET_SCROLL:
                return data[hwnd].nowc;
                case h::constGlobalData::SCROLL::SET:
                data[hwnd].start=((struct h::scrollData*)lp)->start;
                data[hwnd].end=((struct h::scrollData*)lp)->end;
                data[hwnd].now=((struct h::scrollData*)lp)->now;
                data[hwnd].is_move=((struct h::scrollData*)lp)->is_move;
                data[hwnd].nowc=((struct h::scrollData*)lp)->nowc;
                break;
                case h::constGlobalData::SCROLL::SET_END:
                data[hwnd].end=lp;
                break;
            }
        break;
    }
    return DefWindowProc(hwnd,msg,wp,lp);
}
LRESULT CALLBACK settingProc(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp){
    static HWND alpha,r,g,b,fontList;
    static h::ResizeManager rm;
    constexpr int MSG1=1;
    PAINTSTRUCT ps;
    HDC hdc;
    HBRUSH defb,colorb;
    RECT rect;
    switch(msg){
        case WM_DESTROY:
            h::INI(h::constGlobalData::SETTING_FILE)
            .editValue(h::constGlobalData::SECTION_SHOW,h::constGlobalData::KEY_ALPHA,h::cast::toString((int)SendMessage(alpha,WM_COMMAND,h::constGlobalData::SCROLL::GET_SCROLL,0)))
            .editValue(h::constGlobalData::SECTION_SHOW,h::constGlobalData::KEY_BORDER_COLOR,h::vecToString(h::cast::toString(SendMessage(r,WM_COMMAND,h::constGlobalData::SCROLL::GET_SCROLL,0),SendMessage(g,WM_COMMAND,h::constGlobalData::SCROLL::GET_SCROLL,0),SendMessage(b,WM_COMMAND,h::constGlobalData::SCROLL::GET_SCROLL,0))," "))
            .editValue(h::constGlobalData::SECTION_SHOW,h::constGlobalData::KEY_FONT,((std::string*)(SendMessage(fontList,WM_COMMAND,h::constGlobalData::LIST::GET_ITEM,SendMessage(fontList,WM_COMMAND,h::constGlobalData::LIST::GET_SELECT_INDEX,0))))->c_str())
            .save();
        break;
        case WM_CREATE:
        GetClientRect(hwnd,&rect);
        alpha=CreateWindowEx(WS_EX_TOPMOST,TEXT(h::constGlobalData::SCROLL_WINDOW),TEXT("Alpha"),WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS,0,0,rect.right/2,rect.bottom/5,hwnd,NULL,LPCREATESTRUCT(lp)->hInstance,NULL);
        r=CreateWindowEx(WS_EX_TOPMOST,TEXT(h::constGlobalData::SCROLL_WINDOW),TEXT("r"),WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS,0,rect.bottom/5,rect.right/2,rect.bottom/5,hwnd,NULL,LPCREATESTRUCT(lp)->hInstance,NULL);
        g=CreateWindowEx(WS_EX_TOPMOST,TEXT(h::constGlobalData::SCROLL_WINDOW),TEXT("g"),WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS,0,(rect.bottom/5)*2,rect.right/2,rect.bottom/5,hwnd,NULL,LPCREATESTRUCT(lp)->hInstance,NULL);
        b=CreateWindowEx(WS_EX_TOPMOST,TEXT(h::constGlobalData::SCROLL_WINDOW),TEXT("b"),WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS,0,(rect.bottom/5)*3,rect.right/2,rect.bottom/5,hwnd,NULL,LPCREATESTRUCT(lp)->hInstance,NULL);
        fontList=CreateWindow(TEXT(h::constGlobalData::SIMPLELIST_WINDOW),TEXT(""),WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS,rect.right/2,0,rect.right/2,rect.bottom/5*4,hwnd,NULL,LPCREATESTRUCT(lp)->hInstance,NULL);
        {
            h::scrollData data{0,255,0,0,false};
            SendMessage(alpha,WM_COMMAND,h::constGlobalData::SCROLL::SET,(LPARAM)&data);
            SendMessage(r,WM_COMMAND,h::constGlobalData::SCROLL::SET,(LPARAM)&data);
            SendMessage(g,WM_COMMAND,h::constGlobalData::SCROLL::SET,(LPARAM)&data);
            SendMessage(b,WM_COMMAND,h::constGlobalData::SCROLL::SET,(LPARAM)&data);
            h::listData listData{{},0,10,MSG1,NULL};
            h::setFontList(&listData.list);
            SendMessage(fontList,WM_COMMAND,h::constGlobalData::LIST::SET_LIST,LPARAM(&listData));           
            
        }
        h::global::hwnds.clear();
        EnumChildWindows(hwnd,addGlobalHwndsChild,0);
        rm=std::move(h::ResizeManager(hwnd,h::global::hwnds));
        break;
        case WM_PAINT:
        GetClientRect(hwnd,&rect);
        hdc=BeginPaint(hwnd,&ps);
        colorb=CreateSolidBrush(RGB(SendMessage(r,WM_COMMAND,h::constGlobalData::SCROLL::GET_SCROLL,0),SendMessage(g,WM_COMMAND,h::constGlobalData::SCROLL::GET_SCROLL,0),SendMessage(b,WM_COMMAND,h::constGlobalData::SCROLL::GET_SCROLL,0)));
        defb=(HBRUSH)SelectObject(hdc,colorb);
        FrameRect(hdc,&rect,colorb);
        SelectObject(hdc,defb);
        DeleteObject(colorb);
        EndPaint(hwnd,&ps);
        break;
        case WM_SIZE:
        rm.resize();
        break;
        case WM_COMMAND:
            switch(wp){
                case h::constGlobalData::SCROLL::CHANGE:
                    if(HWND(lp)!=alpha){
                        InvalidateRect(hwnd,NULL,TRUE);
                        UpdateWindow(hwnd);
                    }
                break;
                case MSG1:
                break;
            }
        break;
    }
    return DefWindowProc(hwnd,msg,wp,lp);
}
LRESULT CALLBACK listProc(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp){
    static std::unordered_map<HWND,h::listData> data;
    static std::string str;
    RECT rect;
    HDC hdc;
    PAINTSTRUCT ps;
    int oneSize;
    switch(msg){
        case WM_CREATE:
        data[hwnd];
        break;
        case WM_LBUTTONDOWN:
        GetClientRect(hwnd,&rect);
        oneSize=static_cast<double>(rect.bottom)/data[hwnd].showItem;
        data[hwnd].active=(MAKEPOINTS(lp).y/oneSize)+SendMessage(data[hwnd].scroll,WM_COMMAND,h::constGlobalData::SCROLL::GET_SCROLL,0)-1;
        InvalidateRect(hwnd,NULL,TRUE);
        UpdateWindow(hwnd);
        break;
        case WM_LBUTTONDBLCLK:
        if(GetParent(hwnd)==NULL)break;
        SendMessage(GetParent(hwnd),WM_COMMAND,MAKEWPARAM(data[hwnd].msg,data[hwnd].msg),0);
        break;
        case WM_PAINT:
        GetClientRect(hwnd,&rect);
        hdc=BeginPaint(hwnd,&ps);
        SetTextColor(hdc,h::global::borderBrush.getBase());
        SetBkColor(hdc,h::global::bkBrush.getBase());
        {
            oneSize=static_cast<double>(rect.bottom)/data[hwnd].showItem;
            SelectObject(hdc,h::global::font.setHeight(oneSize).getCreated());
            for(int hash=SendMessage(data[hwnd].scroll,WM_COMMAND,h::constGlobalData::SCROLL::GET_SCROLL,0),i=1;hash<data[hwnd].list.size()&&i<data[hwnd].showItem;++hash,++i){
                if(hash==data[hwnd].active){
                    SetTextColor(hdc,h::global::bkBrush.getBase());
                    SetBkColor(hdc,h::global::borderBrush.getBase());
                    TextOut(hdc,0,oneSize*i,data[hwnd].list[hash].c_str(),data[hwnd].list[hash].size());
                    SetTextColor(hdc,h::global::borderBrush.getBase());
                    SetBkColor(hdc,h::global::bkBrush.getBase());
                    continue;
                }
                TextOut(hdc,0,oneSize*i,data[hwnd].list[hash].c_str(),data[hwnd].list[hash].size());
            }
            SelectObject(hdc,GetStockObject(SYSTEM_FONT));
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
        break;
        case WM_COMMAND:
        switch(wp){
            case h::constGlobalData::LIST::SET_LIST:
            data[hwnd].active=((struct h::listData*)lp)->active;
            data[hwnd].list=((struct h::listData*)lp)->list;
            data[hwnd].showItem=((struct h::listData*)lp)->showItem;
            data[hwnd].msg=((struct h::listData*)lp)->msg;
            {
                GetClientRect(hwnd,&rect);
                oneSize=static_cast<double>(rect.bottom)/data[hwnd].showItem;
                if(data[hwnd].scroll==NULL)
                data[hwnd].scroll=CreateWindowEx(WS_EX_TOPMOST,TEXT(h::constGlobalData::SCROLL_WINDOW),TEXT("Height"),WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS,0,0,rect.right,oneSize,hwnd,NULL,(HINSTANCE)GetModuleHandle(0),NULL);
                h::scrollData scrollData{0,static_cast<int>(data[hwnd].list.size()),0,0,false};
                SendMessage(data[hwnd].scroll,WM_COMMAND,h::constGlobalData::SCROLL::SET,(LPARAM)&scrollData);
            }
            break;
            case h::constGlobalData::LIST::PUSH:
                data[hwnd].list.push_back(((std::string*)lp)->c_str());
                SendMessage(data[hwnd].scroll,WM_COMMAND,h::constGlobalData::SCROLL::SET_END,data[hwnd].list.size());
                InvalidateRect(hwnd,NULL,TRUE);
                UpdateWindow(hwnd);
            break;
            case h::constGlobalData::LIST::GET_SELECT_INDEX:
                if(0>data[hwnd].active||data[hwnd].list.size()<=data[hwnd].active){
                    return (LONG)0;
                }
                return (LONG)data[hwnd].active;
            break;
            case h::constGlobalData::LIST::GET_ITEM:
                if(0>data[hwnd].active||data[hwnd].list.size()<=data[hwnd].active){
                    return (LONG)&str;
                }
            return (LONG)&data[hwnd].list[lp];
            break;
            case h::constGlobalData::LIST::GET_OBJ:
                return (LONG)&data[hwnd];
            break;
            case h::constGlobalData::LIST::DELETE_ITEM:
                if(0>lp||data[hwnd].list.size()<=lp){
                    break;
                }
                data[hwnd].list.erase(data[hwnd].list.begin()+lp);
                SendMessage(data[hwnd].scroll,WM_COMMAND,h::constGlobalData::SCROLL::SET_END,data[hwnd].list.size());
                InvalidateRect(hwnd,NULL,TRUE);
                UpdateWindow(hwnd);
            break;
            case h::constGlobalData::SCROLL::CHANGE:
                InvalidateRect(hwnd,NULL,TRUE);
                UpdateWindow(hwnd);
            break;
        }
        break;
    }
    return DefWindowProc(hwnd,msg,wp,lp);
}
LRESULT CALLBACK btnProc(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp){
    PAINTSTRUCT ps;
    HDC hdc;
    RECT rect;
    HBRUSH defb;
    static std::unordered_map<HWND,h::btnData> data;
    switch(msg){
        case WM_CREATE:
        data[hwnd];
        break;
        case WM_LBUTTONDOWN:
        data[hwnd].flag=true;
        InvalidateRect(hwnd,NULL,TRUE);
        UpdateWindow(hwnd);
        break;
        case WM_LBUTTONUP:
        data[hwnd].flag=false;
        InvalidateRect(hwnd,NULL,TRUE);
        UpdateWindow(hwnd);
        if(GetParent(hwnd)==NULL)break;
        SendMessage(GetParent(hwnd),WM_COMMAND,MAKEWPARAM(data[hwnd].msg,0),0);
        break;
        case WM_PAINT:
        GetClientRect(hwnd,&rect);
        hdc=BeginPaint(hwnd,&ps);
        {
        SetTextColor(hdc,h::customBool(data[hwnd].flag,h::global::bkBrush.getBase(),h::global::borderBrush.getBase()));
        SetBkColor(hdc,h::customBool(data[hwnd].flag,h::global::borderBrush.getBase(),h::global::bkBrush.getBase()));
        defb=HBRUSH(SelectObject(hdc,h::customBool(data[hwnd].flag,h::global::borderBrush.getCreated(),h::global::bkBrush.getCreated())));
        }
        Rectangle(hdc,0,0,rect.right,rect.bottom);
        SelectObject(hdc,h::global::font.setHeight(rect.bottom/2).getCreated());
        DrawText(hdc,h::getWindowStr(hwnd).c_str(),-1,&rect,DT_CENTER|DT_WORDBREAK|DT_VCENTER);
        FrameRect(hdc,&rect,h::customBool(data[hwnd].flag,h::global::bkBrush.getCreated(),h::global::borderBrush.getCreated()));
        SelectObject(hdc,defb);
        EndPaint(hwnd,&ps);
        break;
        case WM_COMMAND:
        switch(wp){
            case h::constGlobalData::BTN::SET_BTN:
            data[hwnd].flag=((struct h::btnData*)lp)->flag;    
            data[hwnd].msg=((struct h::btnData*)lp)->msg;
            break;
        }
        break;
    }
    return DefWindowProc(hwnd,msg,wp,lp);
}
namespace h{
    namespace global{
    POINT now,
          hash;
    };
};
VOID CALLBACK resizeRBProc(HWND hwnd , UINT uMsg ,UINT idEvent , DWORD dwTime) {
    if(0<=GetAsyncKeyState(VK_LBUTTON)){
        KillTimer(hwnd,idEvent);
    }
    POINT pos;
    GetCursorPos(&pos);
    MoveWindow(hwnd,h::global::now.x,h::global::now.y,pos.x-h::global::now.x+1,pos.y-h::global::now.y+1,TRUE);
}
VOID CALLBACK resizeLTProc(HWND hwnd , UINT uMsg ,UINT idEvent , DWORD dwTime) {
    if(0<=GetAsyncKeyState(VK_LBUTTON)){
        KillTimer(hwnd,idEvent);
    }
    POINT pos;
    GetCursorPos(&pos);
    MoveWindow(hwnd,pos.x,pos.y,h::global::now.x-pos.x,h::global::now.y-pos.y,TRUE);
}
VOID CALLBACK moveProc(HWND hwnd , UINT uMsg ,UINT idEvent , DWORD dwTime) {
    if(0<=GetAsyncKeyState(VK_LBUTTON)){
        KillTimer(hwnd,idEvent);
    }
    POINT pos;
    GetCursorPos(&pos);
    MoveWindow(hwnd,pos.x-h::global::hash.x,pos.y-h::global::hash.y,h::global::now.x,h::global::now.y,TRUE);
}
LRESULT CALLBACK titleProc(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp){
    RECT rect;
    PAINTSTRUCT ps;
    HDC hdc;
    POINT pos;
    constexpr int MOVEID=1;
    enum MSG{
        EXIT=10
    };
    switch(msg){
        case WM_CREATE:
        GetClientRect(hwnd,&rect);
        {
            h::btnData btnData{0,MSG::EXIT};
            SendMessage(CreateWindow(TEXT(h::constGlobalData::SIMPLEBTN_WINDOW),TEXT("X"),WS_VISIBLE|WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS,rect.right/10*9,0,rect.right/10,rect.bottom,hwnd,NULL,LPCREATESTRUCT(lp)->hInstance,NULL),WM_COMMAND,h::constGlobalData::BTN::SET_BTN,(LPARAM)&btnData);
        }
        break;
        case WM_LBUTTONDOWN:
        if(GetParent(hwnd)==NULL)break;
        GetWindowRect(GetParent(hwnd),&rect);
        GetCursorPos(&pos);
        h::global::hash.x=pos.x-rect.left;
        h::global::hash.y=pos.y-rect.top;
        h::global::now.x=rect.right-rect.left;
        h::global::now.y=rect.bottom-rect.top;
        SetTimer(GetParent(hwnd),MOVEID,h::constGlobalData::MOUSE_UPDATA_COUNT,moveProc);
        break;
        case WM_PAINT:
        GetClientRect(hwnd,&rect);
        hdc=BeginPaint(hwnd,&ps);
        FrameRect(hdc,&rect,h::global::borderBrush.getCreated());
        SelectObject(hdc,h::global::font.setHeight(rect.bottom).getCreated());
        SetTextColor(hdc,h::global::borderBrush.getBase());
        SetBkColor(hdc,h::global::bkBrush.getBase());
        DrawText(hdc,h::getWindowStr(hwnd).c_str(),-1,&rect,DT_CENTER|DT_WORDBREAK|DT_VCENTER);
        EndPaint(hwnd,&ps);
        break;
        case WM_RBUTTONDOWN:
            if(GetParent(hwnd)==NULL)break;
            SendMessage(GetParent(hwnd),msg,wp,lp);
        break;
        case WM_COMMAND:
        switch(LOWORD(wp)){
            case MSG::EXIT:
            if(GetParent(hwnd)==NULL)break;
            SendMessage(GetParent(hwnd),WM_CLOSE,0,0);
            break;
        }
        break;
    }
    return DefWindowProc(hwnd,msg,wp,lp);
}

LRESULT CALLBACK menuProc(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp){
    static std::unordered_map<HWND,h::menuData> data;
    HDC hdc;
    PAINTSTRUCT ps;
    RECT rect;
    switch(msg){
        case WM_CREATE:
            data[hwnd];
        break;
        case WM_COMMAND:
            if(wp<data[hwnd].obj.size()){
                ShowWindow(FindWindowEx(hwnd,NULL,TEXT(h::constGlobalData::SIMPLEBTN_WINDOW),data[hwnd].obj[wp].first.c_str()),SW_HIDE);
                break;
            }
            switch(wp){
                case h::constGlobalData::MENU::SET_MENU:
                    GetClientRect(hwnd,&rect);
                    data[hwnd].obj=((struct h::menuData*)lp)->obj;
                    {
                        int i=0,oneSize=rect.right/data[hwnd].obj.size();
                        rect.right=oneSize;
                        for(auto &[section,list]:data[hwnd].obj){
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
                    }
                break;
            }
            if(GetParent(hwnd)==NULL)break;
            SendMessage(GetParent(hwnd),WM_COMMAND,wp,0);
            h::global::hwnds.clear();
            EnumChildWindows(hwnd,addGlobalHwndsChild,0);
            for(auto hw:h::global::hwnds){
                ShowWindow(hw,SW_SHOW);
            }
        break;
    }
    return DefWindowProc(hwnd,msg,wp,lp);
}
LRESULT CALLBACK WndProc(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp){
    enum MSG{
        Add=10,
        Sub,
        LIST_DBK,
        MENU_FILE_OPEN,
        MENU_FILE_SAVE,
        MENU_HEIP
    };
    enum TIMER{
        RESIZE_RB,
        RESIZE_LT
    };
    constexpr auto CELEND="[CELEND]",
                   REPLACE_ENTER="[ENTER]",
                   SECTION_STRING="STRING",
                   KEY_WORDLIST="wordList",
                   HELPHTML="EasyHelp.html";

    static HWND list,
                edit,
                sub,
                title;
    static OPENFILENAME ofn{0};
    static TCHAR path[MAX_PATH];
    static h::ResizeManager rm;
    PAINTSTRUCT ps;
    HDC hdc;
    std::string str;
    RECT rect;
    POINT pos;
    switch(msg){
        case WM_DESTROY:
            GetWindowRect(hwnd,&rect);
            h::INI(h::constGlobalData::SETTING_FILE)
            .editValue(SECTION_STRING,KEY_WORDLIST,h::vecToString(((struct h::listData*)(SendMessage(list,WM_COMMAND,h::constGlobalData::LIST::GET_OBJ,0)))->list,CELEND))
            .editValue(h::constGlobalData::SECTION_POS,MAINWINDOWNAME,h::vecToString(h::cast::toString(rect.left,rect.top,rect.right-rect.left,rect.bottom-rect.top)," "))
            .save();
            PostQuitMessage(0);
            
        break;
        case WM_CTLCOLOREDIT:
        {   
            SetTextColor(HDC(wp),h::global::borderBrush.getBase());
            SetBkColor(HDC(wp),h::global::bkBrush.getBase());
        }
        return (LONG)h::global::bkBrush.getCreated();
        break;
        case WM_SIZE:
            rm.resize();
        break;

        case WM_RBUTTONDOWN:
            h::windowLong(hwnd,WS_EX_NOACTIVATE,h::customBool(h::getWindowStr(title).compare(h::constGlobalData::MODE_INPUT),h::modeOperator::SUB,h::modeOperator::ADD),true);
            SetWindowText(title,h::customBool(h::getWindowStr(title).compare(h::constGlobalData::MODE_INPUT),TEXT(h::constGlobalData::MODE_INPUT),TEXT(h::constGlobalData::MODE_OUTPUT)));
            InvalidateRect(title,NULL,TRUE);
            UpdateWindow(title);
        break;

        case WM_CREATE:
            
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
            title=CreateWindow(TEXT("title"),TEXT(h::constGlobalData::MODE_INPUT),WS_VISIBLE|WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS,rect.left,rect.top,rect.right-rect.left,(rect.bottom-rect.top)/10,hwnd,NULL,LPCREATESTRUCT(lp)->hInstance,NULL);
            rect.top+=(rect.bottom-rect.top)/10;;
            {
                h::menuData menuData{
                    {
                        {"File",
                            {
                                {"Open",MSG::MENU_FILE_OPEN},
                                {"Save",MSG::MENU_FILE_SAVE}
                            }
                        },
                        {"Help",
                            {
                                {"Open",MSG::MENU_HEIP}
                            }
                        }
                    }
                };
                SendMessage(CreateWindow(TEXT("MENU"),TEXT(""),WS_VISIBLE|WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS,rect.left,rect.top,rect.right-rect.left,(rect.bottom-rect.top)/10,hwnd,NULL,LPCREATESTRUCT(lp)->hInstance,NULL),WM_COMMAND,h::constGlobalData::MENU::SET_MENU,(LPARAM)&menuData);
                rect.top+=(rect.bottom-rect.top)/10;
                h::btnData btnData{false,MSG::Add};
                SendMessage(CreateWindow(TEXT(h::constGlobalData::SIMPLEBTN_WINDOW),TEXT("Add"),BS_PUSHBUTTON|WS_VISIBLE|WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|BS_MULTILINE,((rect.right-rect.left)/10)*8,rect.top,rect.right-((rect.right-rect.left)/10)*8,(rect.bottom-rect.top)/2,hwnd,(HMENU)MSG::Add,LPCREATESTRUCT(lp)->hInstance,NULL),WM_COMMAND,h::constGlobalData::BTN::SET_BTN,(LPARAM)&btnData);
                btnData.msg=MSG::Sub;
                sub=CreateWindow(TEXT(h::constGlobalData::SIMPLEBTN_WINDOW),TEXT("Sub"),BS_PUSHBUTTON|WS_VISIBLE|WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|BS_MULTILINE,((rect.right-rect.left)/10)*8,rect.top+(rect.bottom-rect.top)/2,rect.right-((rect.right-rect.left)/10)*8,(rect.bottom-rect.top)/2,hwnd,(HMENU)MSG::LIST_DBK,LPCREATESTRUCT(lp)->hInstance,NULL);
                SendMessage(sub,WM_COMMAND,h::constGlobalData::BTN::SET_BTN,(LPARAM)&btnData);
                h::listData listData{h::split(h::INI(h::constGlobalData::SETTING_FILE).getData<h::INIT::keyT>(SECTION_STRING,KEY_WORDLIST),CELEND),0,10,MSG::LIST_DBK,NULL};
                list=CreateWindow(TEXT(h::constGlobalData::SIMPLELIST_WINDOW),TEXT(""),BS_PUSHBUTTON|WS_VISIBLE|WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|BS_MULTILINE,rect.left,rect.top,((rect.right-rect.left)/10)*8-rect.left,(rect.bottom-rect.top)/2,hwnd,NULL,LPCREATESTRUCT(lp)->hInstance,NULL);
                SendMessage(list,WM_COMMAND,h::constGlobalData::LIST::SET_LIST,(LPARAM)&listData);
            }
            edit=CreateWindow(TEXT("EDIT"),TEXT(""),ES_AUTOHSCROLL|ES_AUTOVSCROLL|WS_HSCROLL|WS_VSCROLL|WS_VISIBLE|WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|ES_MULTILINE|ES_WANTRETURN,rect.left,rect.top+(rect.bottom-rect.top)/2,((rect.right-rect.left)/10)*8-rect.left,(rect.bottom-rect.top)/2,hwnd,NULL,LPCREATESTRUCT(lp)->hInstance,NULL);
            SendMessage(edit,WM_SETFONT,(WPARAM)h::global::font.setHeight(20).getCreated(),MAKELPARAM(FALSE,0));
            ofn.lStructSize=sizeof(OPENFILENAME);
            ofn.hwndOwner=hwnd;
            ofn.lpstrFilter=TEXT("All files {*.*}\0*.*\0\0");
            ofn.lpstrCustomFilter=0;
            ofn.lpstrFile=path;
            ofn.nMaxFile=MAX_PATH;
            ofn.Flags=OFN_NOVALIDATE;
            GetWindowRect(hwnd,&rect);
            EnumChildWindows(hwnd,addGlobalHwndsChild,0);
            rm=std::move(h::ResizeManager(hwnd,h::global::hwnds));
            break;
        case WM_PAINT:
        GetClientRect(hwnd,&rect);
        hdc=BeginPaint(hwnd,&ps);
        FrameRect(hdc,&rect,h::global::borderBrush.getCreated());
        EndPaint(hwnd,&ps);
        break;
        case WM_LBUTTONDOWN:
        GetWindowRect(list,&rect);
        GetCursorPos(&h::global::now);
        if(h::global::now.x<rect.left||h::global::now.y<rect.top){
            GetWindowRect(hwnd,&rect);
            h::global::now.x=rect.right;
            h::global::now.y=rect.bottom;
            SetTimer(hwnd,TIMER::RESIZE_LT,h::constGlobalData::MOUSE_UPDATA_COUNT,resizeLTProc);        
            break;
        }
        GetWindowRect(sub,&rect);
        if(rect.right<h::global::now.x||rect.bottom<h::global::now.y){
            GetWindowRect(hwnd,&rect);
            h::global::now.x=rect.left;
            h::global::now.y=rect.top;
            SetTimer(hwnd,TIMER::RESIZE_RB,h::constGlobalData::MOUSE_UPDATA_COUNT,resizeRBProc);
            break;
        }
        break;
        case WM_COMMAND:
            switch(LOWORD(wp)){
                case MSG::Add:
                    str=h::replaceAll(h::getWindowStr(edit),"\r\n",REPLACE_ENTER);
                    SendMessage(list,WM_COMMAND,h::constGlobalData::LIST::PUSH,(LPARAM)&str);
                break;
                case MSG::Sub:
                    SendMessage(list,WM_COMMAND,h::constGlobalData::LIST::DELETE_ITEM,SendMessage(list,WM_COMMAND,h::constGlobalData::LIST::GET_SELECT_INDEX,0));
                break;
                case MSG::LIST_DBK:
                    h::pressKeyAll(h::stringToWstring(h::replaceAll(((std::string*)SendMessage(list,WM_COMMAND,h::constGlobalData::LIST::GET_ITEM,SendMessage(list,WM_COMMAND,h::constGlobalData::LIST::GET_SELECT_INDEX,0)))->c_str(),REPLACE_ENTER,h::constGlobalData::ENTER)));
                break;
                case MSG::MENU_FILE_OPEN:
                    if(!GetOpenFileName(&ofn))break;
                    {
                    auto l=((struct h::listData*)(SendMessage(list,WM_COMMAND,h::constGlobalData::LIST::GET_OBJ,0)))->list;
                    if(MessageBox(hwnd,TEXT("reset?"),TEXT("Question"),MB_ICONQUESTION|MB_YESNO)==IDNO){
                            auto a=h::split(h::File(path).read().getContent(),CELEND);
                            l.insert(l.end(),a.begin(),a.end());
                    }
                    else{
                        l=h::split(h::File(path).read().getContent(),CELEND);
                    }
                        ((struct h::listData*)(SendMessage(list,WM_COMMAND,h::constGlobalData::LIST::GET_OBJ,0)))->list=l;
                    }
                    InvalidateRect(list,NULL,TRUE);
                    UpdateWindow(list);
                break;
                case MSG::MENU_FILE_SAVE:
                    if(!GetOpenFileName(&ofn))break;
                    h::File(path).write(h::vecToString(((struct h::listData*)(SendMessage(list,WM_COMMAND,h::constGlobalData::LIST::GET_OBJ,0)))->list,CELEND),MessageBox(hwnd,TEXT("reset?"),TEXT("Question"),MB_ICONQUESTION|MB_YESNO)==IDYES);
                break;
                case MSG::MENU_HEIP:
                    if(!std::filesystem::exists(HELPHTML))h::File(HELPHTML).write(
                    "<!DOCTYPE html><html><head><meta charset=\"utf-8\"><title>EasyHelp</title></head><body><h1>モードについて</h1><div>ウィンドウ内を右クリック:モード変更</div><div>ウィンドウのタイトルバーを見るとモードが書いてある</div><div>Mode:Inputの場合は本ソフトウェアにテキストを書き込めます</div><div>Mode:Outputの場合は入力したい場所にフォーカスをあて入力します</div><div>※入力については後述</div><h1>本ソフトウェアそれぞれのウィンドウについて</h1><div>タイトルバーの無いウィンドウがテキストボックスになります</div><div>※テキストボックスは枠の色が変わっていることろをドラッグするとサイズ変更と疑似的な移動ができます</div><div>\"wordList\"は自動入力するデータを保存し自動入力に使われます</div><div>\"Add\"はテキストボックスの内容をリストに追加します</div><div>\"Sub\"は選択されているリストのデータを削除します</div><h1>入力について</h1><div>モードを\"Mode:Output\"に変更し入力したい場所にフォーカスを当てリストをダブルクリックしてください</div><h1>ファイルの読み取りとセーブについて</h1><div>本ソフトウェアのタイトルバー付近にある\"FILE (F)\"について説明します</div><div>\"FILE(F)\"の\"FILE OPEN\"でファイル選択ウィンドウが開きます</div><div>※ファイル選択については省略</div><div>開いたファイルの[CELEND]までを一つのデータとしリストに表示します</div><div>その際にメッセージボックスが開いてリストをリセットするか聞かれるので「はい」を選択とリストがリセットされ「いいえ」を選択するとそのまま追加されます</div><div>\"FILE(F)\"の\"FILE SAVE\"で同じようにファイル選択ウィンドウが開きます</div><div>開くと先ほどと同じようなファイルをリセットするかファイルの最後に追加するかを聞かれます</div><div>※選択後の説明は略</div><h1>設定ファイルについて</h1><div>本ソフトウェアと同じフォルダ（ディレクトリ）内にあるsetting(.ini)について説明します</div><div>このファイルは起動時に同じフォルダ内に見つからない場合新しく作成されます</div><div>それぞれのデータがどのように機能するかは多すぎるので書きません</div></body></html>"
                    );
                    system(std::filesystem::absolute(HELPHTML).string().c_str());
                break;
                case OPENSETTING:
                    CreateWindow(TEXT(h::constGlobalData::SETTING_WINDOW),TEXT(h::constGlobalData::SETTING_WINDOW),WS_VISIBLE|WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN|WS_CLIPSIBLINGS,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,NULL,NULL,(HINSTANCE)GetModuleHandle(0),NULL);
                break;
            }
        break;
    }
    return DefWindowProc(hwnd,msg,wp,lp);
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
    // h::baseStyle(btnListProc,"btnListProc");
    h::baseStyle(menuProc,"MENU");
    auto re=h::StrToInt(h::INI(h::constGlobalData::SETTING_FILE).getData<h::INIT::keyT>(h::constGlobalData::SECTION_POS,MAINWINDOWNAME),4);
    SendMessage(CreateWindowEx(WS_EX_TOPMOST|WS_EX_LAYERED,TEXT(MAIN_WINDOW_CLASS),TEXT(h::constGlobalData::MODE_INPUT),WS_CLIPCHILDREN|WS_VISIBLE|WS_POPUP,re[POS::X],re[POS::Y],re[POS::WIDTH],re[POS::HEIGHT],NULL,NULL,hInstance,NULL),WM_SETICON,ICON_BIG,(LPARAM)LoadIcon(hInstance,h::constGlobalData::ICON_NAME));
    re=h::StrToInt(h::INI(h::constGlobalData::SETTING_FILE).getData<h::INIT::keyT>(h::constGlobalData::SECTION_SHOW,h::constGlobalData::KEY_BORDER_COLOR),3);
    h::global::borderBrush.reset(RGB(re[RGB::R],re[RGB::G],re[RGB::B]));
    re=h::StrToInt(h::INI(h::constGlobalData::SETTING_FILE).getData<h::INIT::keyT>(h::constGlobalData::SECTION_SHOW,h::constGlobalData::KEY_BK_COLOR),3);
    h::global::bkBrush.reset(RGB(re[RGB::R],re[RGB::G],re[RGB::B]));
    h::global::font.reset(h::INI(h::constGlobalData::SETTING_FILE).getData<h::INIT::keyT>(h::constGlobalData::SECTION_SHOW,h::constGlobalData::KEY_FONT));
    while(GetMessage(&msg,NULL,0,0)){
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return msg.wParam;
}
//btn and scroll はmoveprocとかみたいな処理でマウスの管理をする