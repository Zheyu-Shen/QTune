#include "generatescore.h"
#include <algorithm>
#include <cmath>
#include <QDir>
#include <QFile>
#include <QTextStream>

// ChordGenerator implementation
ChordGenerator::ChordGenerator() : rng(std::random_device{}()) {
}

void ChordGenerator::setSeed(unsigned int seed) {
    rng.seed(seed);
}

std::vector<int> ChordGenerator::getChordIntervals(ChordType type) const {
    std::vector<int> intervals;

    // Base intervals for different chord types
    switch (type) {
    case MAJOR:
        intervals = {0, 4, 7}; // Root, major third, perfect fifth
        break;
    case MINOR:
        intervals = {0, 3, 7}; // Root, minor third, perfect fifth
        break;
    case DIMINISHED:
        intervals = {0, 3, 6}; // Root, minor third, diminished fifth
        break;
    case AUGMENTED:
        intervals = {0, 4, 8}; // Root, major third, augmented fifth
        break;
    case DOMINANT_SEVENTH:
        intervals = {0, 4, 7, 10}; // Root, major third, perfect fifth, minor seventh
        break;
    case MAJOR_SEVENTH:
        intervals = {0, 4, 7, 11}; // Root, major third, perfect fifth, major seventh
        break;
    case MINOR_SEVENTH:
        intervals = {0, 3, 7, 10}; // Root, minor third, perfect fifth, minor seventh
        break;
    }

    return intervals;
}

std::vector<double> ChordGenerator::generateArpeggiatedTiming(int noteCount, double startTime,
                                                              double totalDuration, PatternType pattern) const {
    std::vector<double> startTimes;
    double noteSpacing = totalDuration / noteCount;

    for (int i = 0; i < noteCount; i++) {
        if (pattern == ARPEGGIATED_UP) {
            // Ascending arpeggio
            startTimes.push_back(startTime + i * noteSpacing);
        } else if (pattern == ARPEGGIATED_DOWN) {
            // Descending arpeggio
            startTimes.push_back(startTime + (noteCount - 1 - i) * noteSpacing);
        }
    }
    return startTimes;
}

QString chordTypeToString(ChordType type) {
    switch (type) {
    case MAJOR: return "";
    case MINOR: return "m";
    case DIMINISHED: return "dim";
    case AUGMENTED: return "aug";
    case DOMINANT_SEVENTH: return "7";
    case MAJOR_SEVENTH: return "maj7";
    case MINOR_SEVENTH: return "m7";
    default: return "Unknown";
    }
}

ChordType ChordGenerator::randomChordType(int mode) {
    if(mode == 3){
        std::uniform_int_distribution<int> dist(0, 3);
        return static_cast<ChordType>(dist(rng));
    }
    else{
        std::uniform_int_distribution<int> dist(4, 6);
        return static_cast<ChordType>(dist(rng));
    }
}

int ChordGenerator::randomPitch() {
    // Random root note (C4 to C6 range)
    std::uniform_int_distribution<int> noteDist(PITCH_C, PITCH_C+12);
    return noteDist(rng);
}

void ChordGenerator::generateChord(Score& score, int rootPitch, ChordType chordType,
                                   PatternType patternType, double startTime, double duration) {
    // First, get the intervals.
    std::vector<int> intervals = getChordIntervals(chordType);

    // For block chords, all notes start at the same time
    if (patternType == BLOCK_CHORD) {
        for (int interval : intervals) {
            Note note;
            note.pitch = rootPitch + interval;
            note.startTime = startTime;
            note.duration = duration;
            score.addNote(note);
        }
    } else {
        // For arpeggiated chords, notes are played sequentially
        std::vector<double> noteTimes = generateArpeggiatedTiming(
            intervals.size(), startTime, duration, patternType);

        // Duration for each note in the arpeggio (slightly overlapping)
        double noteDuration = duration / intervals.size() * 1.2;

        // Create notes with the appropriate timings
        for (size_t i = 0; i < intervals.size(); i++) {
            Note note;
            note.pitch = rootPitch + intervals[i];
            note.startTime = noteTimes[i];
            note.duration = noteDuration;
            score.addNote(note);
        }
    }
}

Score ChordGenerator::generateTraining(ChordMode chordMode, double tempo) {
    Score score;

    // Duration in seconds for each chord (based on tempo)
    double chordDuration = 60.0 / tempo;
    double noteDuration = chordDuration / 2;

    double time = 0.0;
    switch(chordMode){
    case ONE_NOTE:
        for(int i = 0; i <= 12; i++){
            Note note;
            note.pitch = PITCH_C + i;
            note.startTime = time; time += noteDuration;
            note.duration = noteDuration;
            score.addNote(note);
        }
        for(int i = 12; i >= 0; i--){
            Note note;
            note.pitch = PITCH_C + i;
            note.startTime = time; time += noteDuration;
            note.duration = noteDuration;
            score.addNote(note);
        }
        for(int i = 0; i <= 12; i++){
            for(int j = 0; j <= 12; j++){
                Note note;
                note.pitch = PITCH_C + i;
                note.startTime = time; time += noteDuration;
                note.duration = noteDuration;
                score.addNote(note);
                note.pitch = PITCH_C + j;
                note.startTime = time; time += noteDuration;
                note.duration = noteDuration;
                score.addNote(note);
            }
        }
        break;
    case TWO_NOTES:
        for(int i = 0; i <= 12; i++){
            for(int j = 0; j <= 12; j++){
                if(j == i) continue;
                for(int k = 0; k < 2; k++){
                    Note note1, note2;
                    note1.pitch = PITCH_C + i;
                    note1.duration = noteDuration;
                    note2.pitch = PITCH_C + j;
                    note2.duration = noteDuration;
                    note1.startTime = time; time += noteDuration;
                    note2.startTime = time; time += noteDuration;
                    score.addNote(note1);
                    score.addNote(note2);
                    note1.startTime = time;
                    note2.startTime = time; time += noteDuration * 2;
                    score.addNote(note1);
                    score.addNote(note2);
                }
            }
        }
        break;
    case THREE_NOTES:
        for (int i = 0; i < 12; i++) {
            // Get root note
            int rootPitch = PITCH_C + i;
            ChordType chords[4] = {MAJOR, MINOR, DIMINISHED, AUGMENTED};
            for(int it=0; it<4; it++){
                auto& chord = chords[it];
                generateChord(score, rootPitch, chord, BLOCK_CHORD,
                              (i*32+it*8) * chordDuration, chordDuration);
                generateChord(score, rootPitch, chord, BLOCK_CHORD,
                              (i*32+it*8+1) * chordDuration, chordDuration);
                generateChord(score, rootPitch, chord, BLOCK_CHORD,
                              (i*32+it*8+2) * chordDuration, chordDuration*2);
                generateChord(score, rootPitch, chord, ARPEGGIATED_UP,
                              (i*32+it*8+4) * chordDuration, chordDuration);
                generateChord(score, rootPitch, chord, ARPEGGIATED_DOWN,
                              (i*32+it*8+5) * chordDuration, chordDuration);
                generateChord(score, rootPitch, chord, BLOCK_CHORD,
                              (i*32+it*8+6) * chordDuration, chordDuration*2);
            }
        }
        break;
    case FOUR_NOTES:
        for (int i = 0; i < 12; i++) {
            // Get root note
            int rootPitch = PITCH_C + i;
            ChordType chords[3] = {DOMINANT_SEVENTH, MAJOR_SEVENTH, MINOR_SEVENTH};
            for(int it=0; it<3; it++){
                auto& chord = chords[it];
                generateChord(score, rootPitch, chord, BLOCK_CHORD,
                              (i*24+it*8) * chordDuration, chordDuration);
                generateChord(score, rootPitch, chord, BLOCK_CHORD,
                              (i*24+it*8+1) * chordDuration, chordDuration);
                generateChord(score, rootPitch, chord, BLOCK_CHORD,
                              (i*24+it*8+2) * chordDuration, chordDuration*2);
                generateChord(score, rootPitch, chord, ARPEGGIATED_UP,
                              (i*24+it*8+4) * chordDuration, chordDuration);
                generateChord(score, rootPitch, chord, ARPEGGIATED_DOWN,
                              (i*24+it*8+5) * chordDuration, chordDuration);
                generateChord(score, rootPitch, chord, BLOCK_CHORD,
                              (i*24+it*8+6) * chordDuration, chordDuration*2);
            }
        }
        break;
    }
    return score;
}

std::pair<Score, QString> ChordGenerator::generateExam(int chordMode, double tempo) {
    Score score;

    // Duration in seconds for each chord (based on tempo)
    double chordDuration = 60.0 / tempo * 2; // 2 beats per chord
    double noteDuration = chordDuration / 4;

    //添加标准音A作为提示
    double time = 0.0;
    Note note;
    note.pitch = PITCH_A;
    note.duration = noteDuration;
    note.startTime = time; time += noteDuration;
    score.addNote(note);

    QString answer; int index, index1, index2; ChordType type;
    const QString pitchNames[12] = {
        "C", "C#", "D", "D#", "E", "F",
        "F#", "G", "G#", "A", "A#", "B"
    };
    switch(chordMode){
    case 1:
        note.pitch = randomPitch();
        note.startTime = time;
        score.addNote(note);
        index = (note.pitch - PITCH_C + 12) % 12;
        answer = pitchNames[index];
        break;
    case 2:
        note.pitch = randomPitch();
        note.startTime = time;
        score.addNote(note);
        index1 = note.pitch - PITCH_C + 12;
        note.pitch = randomPitch();
        note.startTime = time;
        score.addNote(note);
        index2 = note.pitch - PITCH_C + 12;
        if(index1 > index2) std::swap(index1, index2);
        index1 %= 12; index2 %= 12;
        answer = pitchNames[index1] + pitchNames[index2];
        break;
    case 3:
        note.pitch = randomPitch();
        index = (note.pitch - PITCH_C + 12) % 12;
        answer = pitchNames[index];
        type = randomChordType(3);
        answer += chordTypeToString(type);
        generateChord(score, note.pitch, type, BLOCK_CHORD,
                      time, chordDuration);
        break;
    case 4:
        note.pitch = randomPitch();
        index = (note.pitch - PITCH_C + 12) % 12;
        answer = pitchNames[index];
        type = randomChordType(4);
        answer += chordTypeToString(type);
        generateChord(score, note.pitch, type, BLOCK_CHORD,
                      time, chordDuration);
        break;
    }

    return std::make_pair(score, answer);
}

Score ChordGenerator::generateSightSinging1(double tempo) {
    Score score;

    double beatDuration = 60.0 / tempo;  // 一拍的时间（秒）
    double time = 0.0;

    std::uniform_int_distribution<int> pitchDist(PITCH_C, PITCH_C + 12); // 一个八度内随机

    while (time < beatDuration * 16) {
        Note note; note.pitch = PITCH_C + 1;
        while(note.getAccidentSymbol() != ""){
            note.pitch = pitchDist(rng);
        }
        note.startTime = time;
        note.duration = beatDuration;
        time += note.duration;
        score.addNote(note);
    }

    return score;
}

Score ChordGenerator::generateSightSinging2(double tempo) {
    Score score;

    double beatDuration = 30.0 / tempo;  // 一拍的时间（秒）
    double time = 0.0;

    std::uniform_int_distribution<int> pitchDist(PITCH_C - 3, PITCH_C + 14); // 一个八度内随机
    std::uniform_int_distribution<int> noteDurationPattern(1, 4);
    bool sharpflag = false;

    while (time < beatDuration * 32) {
        Note note; note.pitch = PITCH_C + 1;
        if(sharpflag){
            while(note.getAccidentSymbol() != ""){
                note.pitch = pitchDist(rng);
            }
        }
        else{
            note.pitch = pitchDist(rng);
            if(note.getAccidentSymbol() != "") sharpflag = true;
        }
        note.startTime = time;
        note.duration = beatDuration * 3;
        while(note.duration == beatDuration * 3){
            note.duration = beatDuration * noteDurationPattern(rng);
        }
        time += note.duration;
        score.addNote(note);
    }

    return score;
}

Score ChordGenerator::generateSightSinging3(double tempo) {
    Score score;

    double beatDuration = 15.0 / tempo;  // 一拍的时间（秒）
    double time = 0.0;

    std::uniform_int_distribution<int> pitchDist(PITCH_C - 4, PITCH_C + 16); // 一个八度内随机
    std::uniform_int_distribution<int> noteDurationPattern(1, 8);

    while (time <= beatDuration * 128) {
        Note note;
        note.pitch = pitchDist(rng);
        note.startTime = time;
        note.duration = beatDuration * 3;
        while(note.duration == beatDuration * 3){
            qDebug() << noteDurationPattern(rng);
            note.duration = beatDuration * noteDurationPattern(rng);
        }
        time += note.duration;
        score.addNote(note);
    }

    return score;
}
