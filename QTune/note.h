
#pragma once

// 一个“音符”结构

#include <QString>
#include <vector>

class Note {
public:
    int track = 0;           // 属于哪个 MIDI track
    int channel;         // MIDI 通道 0–15
    int pitch;           // 音高 (0–127)
    int velocity = 80;        // 力度 (0–127)
    double startTime;    // 开始时间 (秒)
    double duration;     // 时长 (秒)
    QString getSymbol() const{ // 所有获取属性的函数会以get开头
        // Duration thresholds based on standard notation
        if (duration >= 2.0) return "𝅝";       // Whole note
        if (duration >= 1.0) return "𝅗𝅥";       // Half note
        if (duration >= 0.5) return "𝅘𝅥";       // Quarter note
        if (duration >= 0.25) return "𝅘𝅥𝅮";     // Eighth note
        return "𝅘𝅥𝅯";                          // Sixteenth note
    }
    int x, y; //音符在谱面上的位置
    int width; //音符宽度
    bool isStemUp; //尾巴朝上或下
    QString getName() const{ // Note name
        const QString noteNames[12] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
        return noteNames[pitch % 12];
    }
    QString getAccidentSymbol() const{
        int pitchpos = pitch % 12;
        if(pitchpos == 1 || pitchpos == 3 || pitchpos == 6 || pitchpos == 8 || pitchpos == 10){
            return "#";
        }
        else return "";
    }
};

// 一个“乐谱”类，存储所有音符
class Score {
public:
    std::vector<Note> notes;

    void clear() { notes.clear(); }
    void addNote(const Note& n) { notes.push_back(n); }
    void operator+ (Score& other){
        notes.insert(notes.end(), other.notes.begin(), other.notes.end()); // 将 b 的元素添加到 result 的末尾
    }
    // 你还可以加一些按时间排序／分轨分通道筛选等函数
};


