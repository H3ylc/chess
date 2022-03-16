#include "chess_pieces.h"
#include "chess_board.h"

#include <ncurses.h>

#ifndef NDEBUG
#include "log.h"
extern Log gLog;
#endif

LastTurn::LastTurn()
{
    from_x = -1;
    from_y = -1;
    to_x   = -1;
    to_y   = -1;
    piece  = nullptr;
}

void LastTurn::ChangeLastTurn(int old_x, int old_y, int new_x, int new_y,
                              ChessPiece *new_piece)
{
    from_x = old_x;
    from_y = old_y;
    to_x   = new_x;
    to_y   = new_y;
    piece  = new_piece;
}

ChessPiece::ChessPiece(PieceID pid, TeamID tid) : piece_id(pid), team_id(tid)
{
    color_pair_id = team_id == TeamID::White ? 1 : 3;
}

PawnPiece::PawnPiece(TeamID tid) : ChessPiece(PieceID::Pawn, tid) {}

bool PawnPiece::CanMovePiece(int curr_x, int curr_y, int dest_x, int dest_y,
                             ChessPiece *board[8][8], LastTurn &last_turn)
{
    int success = false;
#ifndef NDEBUG
    fprintf(gLog,
            "%s: Type Pawn (curr_x)[%d] (curr_y)[%d] "
            "(dest_x)[%d] (dest_y)[%d] (team_id)[%d]\n",
            __func__, curr_x, curr_y, dest_x, dest_y,
            static_cast<int>(team_id));
#endif
    if (board[dest_y][dest_x]) {
        if (IsTargetCapturable(board[dest_y][dest_x]) &&
            (curr_x - dest_x == -1 || curr_x - dest_x == 1)) {
            if (team_id == TeamID::White) {
                if (curr_y - dest_y == 1)
                    success = true;
            } else {
                if (dest_y - curr_y == 1)
                    success = true;
            }
        }
    } else {
        if (curr_x == dest_x) {
            if (team_id == TeamID::White) {
                if (curr_y - dest_y <= 1 + !has_moved_before &&
                    curr_y - dest_y > 0 && curr_y - 1 >= 0) {
                    if (!board[curr_y - 1][curr_x])
                        success = true;
                }
            } else {
                if (dest_y - curr_y <= 1 + !has_moved_before &&
                    dest_y - curr_y > 0 && curr_y + 1 <= 7) {
                    if (!board[curr_y + 1][curr_x])
                        success = true;
                }
            }
            // En passant
        } else if (last_turn.piece->GetPieceID() == PieceID::Pawn &&
                   (last_turn.from_y - last_turn.to_y == 2 ||
                    last_turn.from_y - last_turn.to_y == -2) &&
                   (last_turn.to_x == dest_x)) {
            if (curr_x - dest_x == -1) {
                delete board[curr_y][curr_x + 1];
                board[curr_y][curr_x + 1] = nullptr;

                success = true;
            } else if (curr_x - dest_x == 1) {
                delete board[curr_y][curr_x - 1];
                board[curr_y][curr_x - 1] = nullptr;

                success = true;
            }
        }
    }

    if (success)
        has_moved_before = true;
#ifndef NDEBUG
    else
        fprintf(gLog,
                "%s: Pawn not moved from position (curr_x)[%d] (curr_y)[%d] to "
                "(dest_x)[%d] (dest_y)[%d] (board[dest_y][dest_x])[%d]\n",
                __func__, curr_x, curr_y, dest_x, dest_y,
                board[dest_y][dest_x] != nullptr);
#endif

    return success;
}

KnightPiece::KnightPiece(TeamID tid) : ChessPiece(PieceID::Knight, tid) {}

bool KnightPiece::CanMovePiece(int curr_x, int curr_y, int dest_x, int dest_y,
                               ChessPiece *board[8][8], LastTurn &last_turn)
{
    bool success = false;

#ifndef NDEBUG
    fprintf(gLog,
            "%s: Type Knight (curr_x)[%d] (curr_y)[%d] "
            "(dest_x)[%d] (dest_y)[%d] (team_id)[%d]\n",
            __func__, curr_x, curr_y, dest_x, dest_y,
            static_cast<int>(team_id));
#endif

    if (!board[dest_y][dest_x] ||
        board[dest_y][dest_x]->GetTeamID() != team_id) {
        if (((curr_x - dest_x == 1 || curr_x - dest_x == -1) &&
             (curr_y - dest_y == 2 || curr_y - dest_y == -2)) ||
            ((curr_x - dest_x == 2 || curr_x - dest_x == -2) &&
             (curr_y - dest_y == 1 || curr_y - dest_y == -1)))
            success = true;
    }

    if (success)
        has_moved_before = true;
#ifndef NDEBUG
    else
        fprintf(
            gLog,
            "%s: Knight not moved from position (curr_x)[%d] (curr_y)[%d] to "
            "(dest_x)[%d] (dest_y)[%d] (board[dest_y][dest_x])[%d]\n",
            __func__, curr_x, curr_y, dest_x, dest_y,
            board[dest_y][dest_x] != nullptr);
#endif

    return success;
}

BishopPiece::BishopPiece(TeamID tid) : ChessPiece(PieceID::Bishop, tid) {}

bool BishopPiece::CanMovePiece(int curr_x, int curr_y, int dest_x, int dest_y,
                               ChessPiece *board[8][8], LastTurn &last_turn)
{
    bool success = false;

    if (!board[dest_y][dest_x] ||
        board[dest_y][dest_x]->GetTeamID() != team_id) {
        if (curr_x - dest_x == curr_y - dest_y ||
            dest_x - curr_x == curr_y - dest_y) {
            int distance =
                curr_x - dest_x > 0 ? curr_x - dest_x : dest_x - curr_x;
            int x_direction = curr_x - dest_x > 0 ? -1 : 1;
            int y_direction = curr_y - dest_y > 0 ? -1 : 1;

            success = true;
            for (int i = 1; i < distance; i++) {
#ifndef NDEBUG
                fprintf(
                    gLog, "%s: (board[%d][%d])[%d]\n", __func__,
                    curr_y + y_direction * i, curr_x + x_direction * i,
                    board[curr_y + y_direction * i][curr_x + x_direction * i] !=
                        nullptr);
#endif
                if (board[curr_y + y_direction * i][curr_x + x_direction * i]) {
                    success = false;
                    break;
                }
            }
        }
    }

    if (success)
        has_moved_before = true;

    return success;
}

RookPiece::RookPiece(TeamID tid) : ChessPiece(PieceID::Rook, tid) {}

bool RookPiece::CanMovePiece(int curr_x, int curr_y, int dest_x, int dest_y,
                             ChessPiece *board[8][8], LastTurn &last_turn)
{
    return true;
}

QueenPiece::QueenPiece(TeamID tid) : ChessPiece(PieceID::Queen, tid) {}

bool QueenPiece::CanMovePiece(int curr_x, int curr_y, int dest_x, int dest_y,
                              ChessPiece *board[8][8], LastTurn &last_turn)
{
    return true;
}

KingPiece::KingPiece(TeamID tid) : ChessPiece(PieceID::King, tid) {}

bool KingPiece::CanMovePiece(int curr_x, int curr_y, int dest_x, int dest_y,
                             ChessPiece *board[8][8], LastTurn &last_turn)
{
    return true;
}

