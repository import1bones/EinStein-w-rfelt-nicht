# Einstein Game Rules

## Overview

Einstein is a strategic board game for two players featuring dice-based movement and tactical positioning. The game combines elements of chance (dice rolls) and strategy (piece positioning and movement decisions).

## Board Setup

### Board Layout
- **Size**: 5×5 grid board with 25 positions
- **Starting Zones**: 
  - **Red Player (Left-Top)**: Upper-left corner positions
  - **Blue Player (Right-Bottom)**: Lower-right corner positions

### Initial Piece Placement
- Each player has **6 square pieces** numbered **1 through 6**
- At game start, players can arrange their pieces freely within their starting zone
- **Red Player**: Occupies the triangular area in the upper-left corner
- **Blue Player**: Occupies the triangular area in the lower-right corner

### Standard Starting Formation
```
 R1 R2 R3  .  .
 R4 R5  .  .  .
 R6  .  .  .  R4
  .  .  .  B6 B5
  .  .  B3 B2 B1
```
Where:
- `R1-R6`: Red player pieces (numbers 1-6)
- `B1-B6`: Blue player pieces (numbers 1-6)
- `.`: Empty positions

## Gameplay

### Turn Sequence
1. Players alternate turns
2. Active player rolls a six-sided die
3. Player moves the piece corresponding to the die number
4. If the corresponding piece is no longer on the board, move the closest available piece

### Movement Rules

#### Red Player Movement (Left-Top)
Red pieces can move in three directions:
- **Right** (→): One square horizontally right
- **Down** (↓): One square vertically down  
- **Diagonal Right-Down** (↘): One square diagonally toward bottom-right

#### Blue Player Movement (Right-Bottom)
Blue pieces can move in three directions:
- **Left** (←): One square horizontally left
- **Up** (↑): One square vertically up
- **Diagonal Left-Up** (↖): One square diagonally toward top-left

### Piece Selection Rules

#### Primary Rule
When a die is rolled, move the piece with the matching number.

#### Substitution Rule
If the piece matching the die number is no longer on the board:
1. Find the **closest available piece** with a number **greater than** the die roll
2. If no greater number exists, find the **closest available piece** with a number **less than** the die roll

#### Examples:
- Die shows **4**, piece 4 is captured → Move piece 5 (closest greater)
- Die shows **4**, pieces 4,5,6 are captured → Move piece 3 (closest lesser)
- Die shows **6**, piece 6 is captured → Move piece 5 (closest lesser, no greater exists)

### Capture Rules

#### Basic Capture
- If a piece moves to a square occupied by another piece, the occupying piece is **captured** and removed from the board
- **Both enemy and friendly pieces can be captured**

#### Strategic Friendly Capture
- Capturing your own pieces is a valid strategy
- Benefits:
  - Increases movement flexibility for remaining pieces
  - Creates tactical opportunities
  - Can prevent opponent from capturing valuable pieces

### Victory Conditions

A player wins by achieving **either** of the following:

#### 1. Territorial Victory
- **Red Player**: Any piece reaches the opponent's starting corner (bottom-right corner position [4,4])
- **Blue Player**: Any piece reaches the opponent's starting corner (top-left corner position [0,0])

#### 2. Elimination Victory  
- Capture all opponent pieces
- Opponent has no pieces remaining on the board

### Game Characteristics

#### No Draws
- Einstein games always result in a decisive victory
- No stalemate or draw conditions exist
- Games continue until one player achieves a victory condition

#### Mandatory Movement
- If a legal move exists, the player must make a move
- Players cannot pass or skip turns

## Strategic Elements

### Opening Strategy
- **Piece Arrangement**: Initial positioning affects early game mobility
- **Balanced vs. Aggressive**: Different starting formations offer various tactical advantages

### Mid-Game Tactics
- **Piece Preservation**: Protecting high-value pieces while maintaining mobility
- **Sacrifice Plays**: Strategic piece sacrifice for positional advantage
- **Board Control**: Controlling key squares and movement paths

### Endgame Considerations
- **Race vs. Elimination**: Deciding between rushing to opponent's corner or eliminating their pieces
- **Piece Management**: Optimal use of remaining pieces based on die probabilities
