#ifndef DRAWSCORE_H
#define DRAWSCORE_H

#include <QImage>
#include <QPainter>
#include <QString>
#include "note.h"

// Constants for score rendering
const int STAFF_HEIGHT = 40;        // Height of a staff
const int LINE_SPACING = 10;  // Distance between each line
const int TRACK_SPACING = 40; // Distance between each track
const int STAFF_SPACING = 80 - TRACK_SPACING; // Distance between each staff
const int NOTE_WIDTH = 12;          // Note head width
const int MARGIN_LEFT = 50;         // Left margin
const int MARGIN_TOP = 80;          // Top margin
const int PIXELS_PER_SECOND = 75;  // Horizontal scaling factor
const int NOTE_FONT_SIZE = 32;      // 音符字体大小
const int ACCIDENTAL_FONT_SIZE = 12; // 变音记号字体大小
const int MAX_LINE_WIDTH = 1300;  // Maximum width per line in pixels

// Convert MIDI pitch to vertical position on staff
int pitchToYPosition(int pitch);

// Calculate note width based on duration
int calculateNoteWidth(double duration);

// Draw staff lines
void drawStaffLines(QPainter& painter, int yPos, int width);

// Draw a note at the specified position
void drawNote(QPainter& painter, const Note& note, const QFont& musicFont);

// Convert Score to an image
QImage scoreToImage(Score& score);

// Draw score and save to file
bool drawScore(Score& score, const QString& outputDir);

#endif // DRAWSCORE_H
