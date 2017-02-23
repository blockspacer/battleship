#include <cassert>
#include <functional>
#include <iostream>
#include <stdexcept>
#include "game.hpp"
#include "ui/user_interface.hpp"
#include "misc/util.hpp"


Game::Game(Role role, const UIFactory &ui_factory, UIType ui_type, Conn_p conn)
    : role_(role), state_(State::initial),
      board_local_(std::make_shared<Board>()),
      board_remote_(std::make_shared<Board>()),
      ui_(ui_factory.make(ui_type, board_local_, board_remote_)),
      game_comm_(conn)
{
    ui_->show();
}

Game::~Game()
{
}

void Game::run()
{
    while (state_ != State::end)
    {
        switch (state_)
        {
            case State::initial:
                handle_initial();
                break;
            case State::ships_placed:
                handle_ships_placed();
                break;
            case State::ships_committed:
                handle_ships_committed();
                break;
            case State::commitments_sent:
                handle_commitments_sent();
                break;
            case State::commitments_received:
                handle_commitments_received();
                break;
            case State::ready:
                handle_ready();
                break;
            case State::query_phase:
                handle_query_phase();
                break;
            case State::answer_phase:
                handle_answer_phase();
                break;
            default:
                throw std::logic_error("unknown state");
        }
    }
    handle_end();
}

void Game::handle_initial()
{
    assert(state_ == State::initial);
    std::cerr << "state: initial\n";
    // place ships
    board_local_->debug();
    ui_->place_ships();
    board_local_->debug();
    state_ = State::ships_placed;
}

void Game::handle_ships_placed()
{
    assert(state_ == State::ships_placed);
    std::cerr << "state: ships_placed\n";
    // TODO: make and exchange commitments
    // state_ = State::ships_committed;
    state_ = State::ready;
}

void Game::handle_ships_committed()
{
    assert(state_ == State::ships_committed);
    std::cerr << "state: ships_committed\n";
    if (role_ == Role::server)
    {
        // send commitments
        state_ = State::commitments_sent;
    }
    else
    {
        // recv commitments
        state_ = State::commitments_received;
    }
}

void Game::handle_commitments_sent()
{
    assert(state_ == State::commitments_sent);
    assert(role_ == Role::server);
    // recv commitments
    state_ = State::ready;
}


void Game::handle_commitments_received()
{
    assert(state_ == State::commitments_received);
    assert(role_ == Role::client);
    // send commitments
    state_ = State::ready;
}

void Game::handle_ready()
{
    assert(state_ == State::ready);
    std::cerr << "state: ready\n";
    if (role_ == Role::server)
        state_ = State::query_phase;
    else
        state_ = State::answer_phase;
}

void Game::handle_query_phase()
{
    assert(state_ == State::query_phase);
    std::cerr << "state: query_phase\n";

    ui_->post_message("select position");
    auto coords{ui_->select_position()};
    auto answer{game_comm_.query_position(coords)};
    board_remote_->set_queried(coords, answer);
    if (board_remote_->num_ships_hit() < 3)
    {
        state_ = State::answer_phase;
    }
    else
    {
        won_ = true;
        state_ = State::end;
    }
}

void Game::handle_answer_phase()
{
    assert(state_ == State::answer_phase);
    std::cerr << "state: answer_phase\n";

    ui_->post_message("answering position");
    game_comm_.answer_query([this] (coords_t coords)
        {
            return this->board_local_->query(coords);
        });
    if (board_local_->ships_alive())
    {
        state_ = State::query_phase;
    }
    else
    {
        won_ = false;
        state_ = State::end;
    }
}

void Game::handle_end()
{
    assert(state_ == State::end);
    std::cerr << "state: end\n";

    if (won_)
    {
        ui_->post_message("You won: you destroyed all enemy ships");
    }
    else
    {
        ui_->post_message("You lost: all your ships are destroyed");
    }
    ui_->wait_for_quit();
}