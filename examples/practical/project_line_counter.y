include sys;
include fs;
include file;

if(sys.args.size() < 3)
{
    eprintln("need argument(s)");
    y = fs.path.name(sys.args[0]);
    eprintln("usage) {y} <directory> <language_type(c,cpp,js,py,cs,y)> [report format(text,csv,json]");
    eprintln("ex1) {y} /toy c");
    eprintln("ex2) {y} dev/ cpp json");
    exit(1);
}

folder = sys.args[1];

langMap = {};
langMap['c'] = ['h', 'c'];
langMap['cpp'] = ['h', 'c', 'hpp', 'cpp', 'hxx', 'cxx', 'inl'];
langMap['js'] = ['js', 'ts', 'jsx', 'tsx'];
langMap['py'] = ['py'];
langMap['cs'] = ['cs'];
langMap['y'] = ['y'];

langType = sys.args[2].tolower();
if(!langMap.contains(langType))
{
    eprintln("'{langType}': not supported language type");
    eprintln("supported types are {langMap.keys()}.");
    exit(1);
}
langExts = langMap[langType];

reportFmt = sys.args.size() > 3 ? sys.args[3].tolower() : 'text';
if(reportFmt != 'text' && reportFmt != 'csv' && reportFmt != 'json')
{
    eprintln("'{reportFmt}': not supported report format");
    eprintln("assume to 'text'");
    reportFmt = 'text';
}

files = fs.readdir(folder, true);

srcList = [];

class SourceInfo
{
    _file = "";
    _byteCnt = 0;
    _lineCnt = 0;
}

for(i=0; i<files.size(); i++)
{
    for(j=0; j<langExts.size(); j++)
    {
        if( files[i].ends_with("." + langExts[j]))
        {
            srcList.push_back(SourceInfo());
            srcList.back()._file = files[i];
            break;
        }
    }
}

for(i=0; i<srcList.size(); i++)
{
    src = file.read_all(srcList[i]._file);
    srcList[i]._byteCnt = src.size();
    srcList[i]._lineCnt = src.split('\n').size();
}

if(reportFmt == 'text')
{
    totalBytes = 0;
    totalLines = 0;
    for(i=0; i<srcList.size(); i++)
    {
        println("{i+1}:{srcList[i]._file} Bytes:{srcList[i]._byteCnt} Lines:{srcList[i]._lineCnt}");
        totalBytes += srcList[i]._byteCnt;
        totalLines += srcList[i]._lineCnt;
    }
    println("total bytes: {totalBytes}, totalLines: {totalLines}");
}
else if(reportFmt == 'csv')
{
    println("File,Bytes,Lines");
    for(i=0; i<srcList.size(); i++)
    {
        println("{srcList[i]._file},{srcList[i]._byteCnt},{srcList[i]._lineCnt}");
    }
}
else
{
    json = { "files":[], "totalBytes":0, "totalLines":0 };

    for(i=0; i<srcList.size(); i++)
    {
        json["files"].push_back({"files": srcList[i]._file, "bytes": srcList[i]._byteCnt, "lines": srcList[i]._lineCnt});
        json["totalBytes"] += srcList[i]._byteCnt;
        json["totalLines"] += srcList[i]._lineCnt;
    }
    println(json);
}
