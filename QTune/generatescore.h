#ifndef GENERATESCORE_H
#define GENERATESCORE_H

#pragma once

#include <vector>
#include <random>
#include "note.h"

// Constants for MIDI pitch values
#define PITCH_C 60
#define PITCH_A 69

// Chord types
enum ChordType
{
    MAJOR,
    MINOR,
    DIMINISHED,
    AUGMENTED,
    DOMINANT_SEVENTH,
    MAJOR_SEVENTH,
    MINOR_SEVENTH
};
QString chordTypeToString(ChordType type);

// Pattern types
enum PatternType
{
    BLOCK_CHORD,     // 柱式和弦 (all notes played simultaneously)
    ARPEGGIATED_UP,  // 分解和弦 (ascending)
    ARPEGGIATED_DOWN // 分解和弦 (descending)
};

// Voice mode (number of notes in chord)
enum ChordMode
{
    ONE_NOTE = 1,    // Single note
    TWO_NOTES = 2,   // Two notes (e.g., intervals)
    THREE_NOTES = 3, // Three-note chords (triads)
    FOUR_NOTES = 4   // Four-note chords (seventh chords)
};

// Chord generator class
class ChordGenerator
{
private:
    std::mt19937 rng;

    // Helper methods
    std::vector<int> getChordIntervals(ChordType type) const;
    std::vector<double> generateArpeggiatedTiming(int noteCount, double startTime,
                                                  double totalDuration, PatternType pattern) const;

public:
    ChordGenerator();

    // Set a specific seed for reproducible results
    void setSeed(unsigned int seed);

    // Generate random elements
    ChordType randomChordType(int mode);
    int randomPitch();

    // Generate a chord and add it to the score
    void generateChord(Score &score, int rootPitch, ChordType chordType,
                       PatternType patternType, double startTime, double duration);

    // Generate a complete exercise with multiple chords
    Score generateTraining(ChordMode chordMode, double tempo = 60.0);

    // Generate an exam with answer validation
    std::pair<Score, QString> generateExam(int chordMode, double tempo = 60.0);

    Score generateSightSinging1(double tempo = 70.0);
    Score generateSightSinging2(double tempo = 70.0);
    Score generateSightSinging3(double tempo = 70.0);
};

#endif // GENERATESCORE_H
