#include "drawscore.h"
#include <QApplication>
#include <QDir>
#include <QFont>
#include <QFontDatabase>
#include <QMap>
#include <algorithm>
#include <vector>
#include <iostream>

// Convert MIDI pitch to vertical position on staff
int pitchToYPosition(int pitch) {
    // Middle E (MIDI 64) as reference point
    // Each step is LINE_SPACING / 2 (each line or space)
    //return STAFF_HEIGHT - ((pitch - 64) * LINE_SPACING / 2);

    // 创建音高到五线谱位置的映射表
    // 每个位置对应LINE_SPACING/2的间距(线到间，或间到线)
    static const int pitchToStaffPosition[] = {
        // 从C开始的半音序列，对应五线谱上的位置
        // C, C#, D, D#, E, F, F#, G, G#, A, A#, B
           0, 0, 1, 1, 2, 3, 3, 4, 4, 5, 5, 6  // 一个八度内的位置
    };

    // 计算相对于参考音符的八度差异
    int octaveDiff = (pitch - 60) / 12;
    int noteInOctave = (pitch - 60) % 12;
    if (noteInOctave < 0) {
        noteInOctave += 12;
        octaveDiff--;
    }
    // 计算五线谱上的位置
    int staffPosition = pitchToStaffPosition[noteInOctave] + octaveDiff * 7;
    staffPosition -= 3;

    // 转换为Y坐标 (每个位置是LINE_SPACING/2)
    return STAFF_HEIGHT - (staffPosition * LINE_SPACING / 2);
}

// Calculate note width based on duration
int calculateNoteWidth(double duration) {
    // Simple linear scaling based on duration
    return std::max(int(duration * PIXELS_PER_SECOND), NOTE_WIDTH);
}

// Draw staff lines
void drawStaffLines(QPainter& painter, int yPos) {
    painter.setPen(QPen(Qt::black, 1));
    for (int i = 0; i < 5; i++) {
        int y = yPos + i * LINE_SPACING;
        painter.drawLine(MARGIN_LEFT, y, MARGIN_LEFT + MAX_LINE_WIDTH - MARGIN_LEFT * 2, y);
    }

    // Draw treble clef (simplified representation)
    QFont musicFont("Arial", 40);
    painter.setFont(musicFont);
    painter.drawText(MARGIN_LEFT - 40, yPos + 4 * LINE_SPACING, "𝄞");
}

// Draw a note at the specified position
void drawNote(QPainter& painter, const Note& note, int staffY) {
    // Calculate absolute y position based on staff position
    int absoluteY = staffY + note.y;

    // Draw note
    QFont noteFont("Arial", NOTE_FONT_SIZE);
    painter.setFont(noteFont);
    painter.setPen(Qt::black);

    // Draw note head
    QString noteSymbol = note.getSymbol();
    painter.drawText(note.x, absoluteY, noteSymbol);

    // Draw accidental symbol (using smaller font)
    QString accidentSymbol = note.getAccidentSymbol();
    if (!accidentSymbol.isEmpty()) {
        QFont accidentalFont("Arial", ACCIDENTAL_FONT_SIZE);
        painter.setFont(accidentalFont);
        painter.drawText(note.x - 10, absoluteY - 2, accidentSymbol);
    }

    // Draw ledger lines if needed
    int staffTop = staffY;
    int staffBottom = staffY + 4 * LINE_SPACING;
    // Ledger lines above staff
    if (absoluteY < staffTop - LINE_SPACING / 2) {
        int lineY = staffTop - LINE_SPACING;
        while (lineY >= absoluteY - LINE_SPACING / 2) {
            painter.drawLine(note.x - 5, lineY, note.x + 15, lineY);
            lineY -= LINE_SPACING;
        }
    }
    // Ledger lines below staff
    if (absoluteY > staffBottom + LINE_SPACING / 2) {
        int lineY = staffBottom + LINE_SPACING;
        while (lineY <= absoluteY + LINE_SPACING / 2) {
            painter.drawLine(note.x - 5, lineY, note.x + 15, lineY);
            lineY += LINE_SPACING;
        }
    }
}

// Convert Score to an image with multiple lines
QImage scoreToImage(Score& score) {
    // Calculate total duration
    double maxTime = 0.0;
    for (const auto& note : score.notes) {
        maxTime = std::max(maxTime, note.startTime + note.duration);
    }

    // Find tracks
    QMap<int, bool> tracksMap;
    for (const auto& note : score.notes) {
        tracksMap[note.track] = true;
    }
    std::vector<int> tracks;
    for (auto it = tracksMap.begin(); it != tracksMap.end(); ++it) {
        tracks.push_back(it.key());
    }
    std::sort(tracks.begin(), tracks.end());

    // Calculate how much time fits in one line
    int timePerLine = (MAX_LINE_WIDTH - MARGIN_LEFT * 2) / PIXELS_PER_SECOND;
    int numLines = (maxTime + timePerLine - 2) / timePerLine;

    // Calculate image dimensions
    int scoreHeight = MARGIN_TOP + numLines * (tracks.size() * (STAFF_HEIGHT + TRACK_SPACING) + STAFF_SPACING);

    // Create the image
    QImage image(MAX_LINE_WIDTH, scoreHeight, QImage::Format_ARGB32);
    image.fill(Qt::white);

    // Create painter
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing); //减少图像边缘的锯齿状效果，让线条和曲线看起来更加平滑

    // Process each line
    for (int line = 0; line < numLines; line++) {
        double lineStartTime = line * timePerLine;
        double lineEndTime = (line + 1) * timePerLine;

        // Process each track
        for (size_t trackIdx = 0; trackIdx < tracks.size(); trackIdx++) {
            int trackNum = tracks[trackIdx];

            // Calculate Y position for this track on this line
            int nowY = MARGIN_TOP + line * (tracks.size() * (STAFF_HEIGHT + TRACK_SPACING) + STAFF_SPACING)
                       + trackIdx * (STAFF_HEIGHT + TRACK_SPACING);

            // Draw track label (only on first line for each track)
            painter.setFont(QFont("Arial", 12));
            painter.drawText(10, nowY - 5, QString("Track %1").arg(trackNum));

            // Draw staff lines for this section
            drawStaffLines(painter, nowY);

            // Collect and draw notes for this track and time range
            std::vector<Note> lineNotes;
            for (auto note : score.notes) {
                if (note.track == trackNum &&
                    note.startTime >= lineStartTime &&
                    note.startTime < lineEndTime) {

                    // Adjust note position relative to this line
                    note.x = MARGIN_LEFT + int((note.startTime - lineStartTime) * PIXELS_PER_SECOND);
                    note.y = pitchToYPosition(note.pitch);
                    note.width = calculateNoteWidth(note.duration);
                    note.isStemUp = (note.pitch < 70);

                    lineNotes.push_back(note);
                }
            }

            // Draw all notes for this track and line
            for (const auto& note : lineNotes) {
                drawNote(painter, note, nowY);
            }

            // Draw measure lines (optional - every 4 beats or similar)
            painter.setPen(QPen(Qt::gray, 1, Qt::DashLine));
            double measureDuration = 4.0; // 4 seconds per measure (adjust as needed)
            for (double t = lineStartTime; t < lineEndTime; t += measureDuration) {
                if (t > lineStartTime) { // Don't draw at the very beginning
                    int x = MARGIN_LEFT + int((t - lineStartTime) * PIXELS_PER_SECOND);
                    painter.drawLine(x, nowY, x, nowY + STAFF_HEIGHT);
                }
            }
        }
    }

    painter.end();
    return image;
}

// draw score and save to file
bool drawScore(Score& score, const QString& outputDir) {
    // Create output directory if it doesn't exist
    QDir dir;
    if (!dir.exists(outputDir)) {
        dir.mkpath(outputDir);
    }

    // Generate the score image
    QImage scoreImage = scoreToImage(score);

    // Save the image
    QString outputPath = outputDir + "/tmp.png";
    bool success = scoreImage.save(outputPath);

    if (success) {
        std::cout << "Score image saved successfully to: " << outputPath.toStdString() << std::endl;
    } else {
        std::cerr << "Failed to save score image to: " << outputPath.toStdString() << std::endl;
    }

    return success;
}
