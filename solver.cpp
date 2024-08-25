#include <iterator>
#include <numeric>
#include <functional>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <cstring>
#include <string>
#include <climits>
#include <unordered_map>
#include <unordered_set>
#include <random>
#include <cassert>
#include <deque>
#include <stack>
#include <queue>
#include <cmath>
#include <list>
#include <map>
#include <set>


const int DEBUG = 0;

const int SOUTH = 0;
const int WEST = 1;
const int EAST = 2;


const int SPADES = 0;
const int CLUBS = 1;
const int DIAMONDS = 2;
const int HEARTS = 3;

const std::vector<std::string> PLAYERS = {"South", "West", "East"};

int main_suit = -1;

const std::string cards = "7890JQKA";


std::map<std::vector<unsigned int>, std::pair<int, int> > cache;


int create_hand(std::array<std::string, 4>& suits)
{
    unsigned int result = 0;
    for(size_t suit = 0; suit < 4; ++suit) {
        for(const char ch: suits.at(suit)) {

            int idx = cards.find(ch);
            assert(idx >= 0);
            result |= (1 << (idx + suit * 8));
        }
    }

    return result;
}


int get_suit(int card) {
    return card / 8;
}


void update(int& result, int& best_card, int res, int card)
{
    if (res > result) {
        result = res;
        best_card = card + 1;
    }
}


void show_card(int idx) {
    --idx;

    int suit = idx / 8;
    if (suit == SPADES) {
        std::cout << "♠";
    }
    if (suit == CLUBS) {
        std::cout << "♣";
    }
    if (suit == DIAMONDS) {
        std::cout << "♥";
    }
    if (suit == HEARTS) {
        std::cout << "♦";
    }

    std::cout << cards[idx % 8];
}

/* card is in [0, 32) range */
int who(const int first_card, const int second_card, const int third_card, const int main_suit)
{
    const int first_suit = first_card / 8;
    const int second_suit = second_card / 8;
    const int third_suit = third_card / 8;

    const int first_rank = first_card % 8;
    const int second_rank = second_card % 8;
    const int third_rank = third_card % 8;


    bool has_main_suit = first_suit == main_suit || second_suit == main_suit || third_suit == main_suit;

    if (has_main_suit) {
        if (first_suit == second_suit && first_suit == third_suit) {
            if (first_rank > second_rank && first_rank > third_rank)
                return 0;
            else if (first_rank < second_rank && second_rank > third_rank)
                return 1;
            else if (third_rank > second_rank && first_rank < third_rank)
                return 2;
            else
                assert(false);
        }
        else {
            if (first_suit == main_suit && second_suit == main_suit) {
                return first_rank > second_rank ? 0 : 1;
            }
            else if (first_suit == main_suit && third_suit == main_suit) {
                return first_rank > third_rank ? 0 : 2;
            }
            else if (second_suit == main_suit && third_suit == main_suit) {
                return second_rank > third_rank ? 1 : 2;
            }
            else if (first_suit == main_suit) {
                return 0;
            }
            else if (second_suit == main_suit) {
                return 1;
            }
            else if (third_suit == main_suit) {
                return 2;
            }
            else {
                assert(false);
            }
        }

    }

    if (first_suit != second_suit && first_suit != third_suit) {
        return 0;
    }

    if (first_suit == second_suit && first_suit == third_suit) {
        if (first_rank > second_rank && first_rank > third_rank)
            return 0;
        else if (first_rank < second_rank && second_rank > third_rank)
            return 1;
        else if (third_rank > second_rank && first_rank < third_rank)
            return 2;
        else
            assert(false);
    }
    else if (first_suit == second_suit) {
        if (first_rank > second_rank)
            return 0;
        return 1;
    }
    else if (first_suit == third_suit) {
        if (first_rank > third_rank)
            return 0;
        return 2;
    }

    return 0;
}

/*
south, west, east - bitmasks
move is 0, 1 or 2
first_card, second card is in [1, 32]
*/
std::pair<int, int> solve(unsigned int south, unsigned int west, unsigned int east, unsigned int move, unsigned int first_card, unsigned int second_card)
{
    unsigned int& current_hand = move == 0 ? south : (move == 1 ? west : east);

    if (current_hand == 0)
        return std::make_pair(0, 0);

    std::vector<unsigned int> key = {south, west, east, move, first_card, second_card};
    std::pair result_pair = cache[key];

    int& result = result_pair.first;
    int& best_card = result_pair.second;

    if (best_card != 0)
        return result_pair;

    result = -1;

    if (first_card == 0)
    {
        int previous = -2;
        for(unsigned int i = 0; i < 32; ++i) {
            if (!(current_hand & (1u << i))) {
                continue;
            }

            if (previous + 1 == i && get_suit(i) == get_suit(previous)) {
                previous = i;
                continue;
            }

            previous = i;

            if (move == SOUTH) {
                auto p = solve(south - (1u << i), west, east, (move + 1) % 3, i + 1, 0);
                int res = __builtin_popcount(current_hand) - p.first;
                update(result, best_card, res, i);
            }
            else if (move == WEST) {
                auto p = solve(south, west - (1u << i), east, (move + 1) % 3, i + 1, 0);
                update(result, best_card, p.first, i);
            }
            else if (move == EAST) {
                auto p = solve(south, west, east - (1u << i), (move + 1) % 3, i + 1, 0);
                int res = __builtin_popcount(current_hand) - p.first;
                update(result, best_card, res, i);
            }
            else {
                std::cout << "Unknown move = " << move << std::endl;
                assert(false);
            }
        }
    }
    else if (second_card == 0) {

        int previous = -2;
        for(unsigned int i = 0; i < 32; ++i) {
            if (!(current_hand & (1u << i))) {
                continue;
            }

            bool has_first_suit = (current_hand >> (8 * get_suit(first_card - 1))) & 0xFFu;
            if (has_first_suit && get_suit(i) != get_suit(first_card - 1)) {
                continue;
            }

            if (previous + 1 == i && get_suit(i) == get_suit(previous)) {
                previous = i;
                continue;
            }

            previous = i;

            if (move == SOUTH) {
                auto p = solve(south - (1u << i), west, east, (move + 1) % 3, first_card, i+1);

                int res = __builtin_popcount(current_hand) - p.first;
                update(result, best_card, res, i);
            }
            else if (move == WEST) {
                auto p = solve(south, west - (1u << i), east, (move + 1) % 3, first_card, i+1);
                update(result, best_card, p.first, i);
            }
            else if (move == EAST) {
                auto p = solve(south, west, east - (1u << i), (move + 1) % 3, first_card, i+1);
                int res = __builtin_popcount(current_hand) - p.first;
                update(result, best_card, res, i);
            }
            else {
                assert(false);
            }
        }
    }
    else if (second_card != 0) {

        assert(first_card != 0);

        int previous = -2;
        for(unsigned int i = 0; i < 32; ++i) {
            if (!(current_hand & (1u << i))) {
                continue;
            }

            bool has_first_suit = (current_hand >> (8 * get_suit(first_card-1))) & 0xFF;
            if (has_first_suit && get_suit(i) != get_suit(first_card - 1)) {
                continue;
            }

            if (previous + 1 == i && get_suit(i) == get_suit(previous)) {
                previous = i;
                continue;
            }
            previous = i;
            
            const int capture = who(first_card-1, second_card-1, i, main_suit);

            if (DEBUG) {
                std::cout << "capture = " << (first_card - 1) << " " << (second_card - 1) << " " << i << ": " << capture << std::endl;
            }

            if (move == SOUTH) {
                // west, east, south

                if (capture == 0) {
                    auto p = solve(south - (1u << i), west, east, EAST, 0, 0);
                    update(result, best_card, __builtin_popcount(east) - (p.first + 1), i);
                } 
                else if (capture == 1) {
                    auto p = solve(south - (1u << i), west, east, WEST, 0, 0);
                    update(result, best_card, __builtin_popcount(west) - (p.first + 1), i);
                } 
                else if (capture == 2) {
                    auto p = solve(south - (1u << i), west, east, SOUTH, 0, 0);
                    update(result, best_card, p.first + 1, i);
                }
                else {
                    std::cout << "Unknown capture = " << capture << std::endl;
                    assert(false);
                }
            }
            else if (move == WEST) {
                // east, south, west

                if (capture == 0) {
                    auto p = solve(south, west - (1u << i), east, EAST, 0, 0);
                    update(result, best_card, p.first + 1, i);
                } 
                else if (capture == 1) {
                    auto p = solve(south, west - (1u << i), east, SOUTH, 0, 0);
                    update(result, best_card, __builtin_popcount(west) - (p.first + 1), i);
                } 
                else if (capture == 2) {
                    auto p = solve(south, west - (1u << i), east, WEST, 0, 0);
                    update(result, best_card, p.first + 1, i);
                }
                else {
                    std::cout << "Unknown capture = " << capture << std::endl;
                    assert(false);
                }
            }
            else if (move == EAST) {
                // south, west, east

                if (capture == 0) {
                    auto p = solve(south, west, east - (1u << i), SOUTH, 0, 0);
                    update(result, best_card, __builtin_popcount(east) - (p.first + 1), i);
                } 
                else if (capture == 1) {
                    auto p = solve(south, west, east - (1u << i), WEST, 0, 0);
                    update(result, best_card, p.first + 1, i);
                } 
                else if (capture == 2) {
                    auto p = solve(south, west, east - (1u << i), EAST, 0, 0);
                    update(result, best_card, p.first + 1, i);
                }
                else {
                    std::cout << "Unknown capture = " << capture << std::endl;
                    assert(false);
                }
            }
        }
    }
    else {
        assert(false);
    }
    
    if (DEBUG) {
        std::cout << south << " " << west << " " << east << " " << move << " " << first_card << " " << second_card << ": " << result << " " << best_card << std::endl;
    }

    return result_pair;
}


int main(int argc, const char * argv[])
{
    main_suit = SPADES;

    std::array<std::string, 4> south_hand = {"AJ87", "", "A", ""};
    std::array<std::string, 4> west_hand = {"K0", "", "K78", ""};
    std::array<std::string, 4> east_hand = {"Q9", "", "QJ0", ""};

    auto south = create_hand(south_hand);
    auto west = create_hand(west_hand);
    auto east = create_hand(east_hand);

    std::cout << PLAYERS[0] << ": " << south << std::endl;
    std::cout << PLAYERS[1] << ": " << west << std::endl;
    std::cout << PLAYERS[2] << ": " << east << std::endl;

    int move = 0;

    int first_card = 0, second_card = 0;
    while(south || west || east) {

        auto p = solve(south, west, east, move, first_card, second_card);
        const unsigned int score = p.first, card = p.second, card_bit = p.second - 1;

        std::cout << "Player: " << PLAYERS[move] << ", score: " << score << ", card: ";
        show_card(card);
        std::cout << std::endl;

        if (move == SOUTH) {
            assert(south & (1u << card_bit));
            south ^= (1u << card_bit);
        }
        else if (move == WEST) {
            assert(west & (1u << card_bit));
            west ^= (1u << card_bit);
        }
        else if (move == EAST) {
            assert(east & (1u << card_bit));
            east ^= (1u << card_bit);
        }
        else {
            assert(false);
        }

        if (first_card == 0) {
            first_card = card;
            move = (move + 1) % 3;
        }
        else if (second_card == 0) {
            second_card = card;
            move = (move + 1) % 3;
        }
        else {

            int capture = who(first_card - 1, second_card - 1, card - 1, main_suit);
            std::cout << "Simulation: " << first_card << " " << second_card << " " << card << " " << capture << std::endl;
            first_card = second_card = 0;

            if (capture == 2) {
                move = move + 0;
            }
            else if (capture == 1) {
                move = (move + 2) % 3;
            }
            else if (capture == 0) {
                move = (move + 1) % 3;
            }
            else {
                assert(false);
            }

            std::cout << "Player captures: " << PLAYERS[move] << std::endl;
        }
    }
    
    return 0;
}