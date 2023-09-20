#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>
#include <random>

#define MAX_BOUNCING_ANGLE_RL 3.
#define MIN_BOUNCING_ANGLE_RL -3.
#define MAX_BOUNCING_ANGLE_TD 12.
#define MIN_BOUNCING_ANGLE_TD -12.

#define WINDOW_X 1800
#define WINDOW_Y 900

#define BALL_RADIUS 20.
#define BALL_START_VELOCITY 220.
#define BALL_START_ANGLE 155.
#define BALL_STEP_VELOCITY 0.005
#define BALL_START_POSITION_X WINDOW_X / 2 - BALL_RADIUS
#define BALL_START_POSITION_Y WINDOW_Y / 2 - BALL_RADIUS

enum class SIDE{
	TOP,
	DOWN,
	RIGHT,
	LEFT,
	NO_COLLISION
	
};

class Buffer_painter
{
	public:
	void paintElements(sf::RenderWindow &w);

	void push_event(sf::RectangleShape &a){
		list_eventr.push_back(a);
	}
		void push_event(sf::CircleShape &a){
		list_eventc.push_back(a);
	}
	private:
	std::vector<sf::RectangleShape> list_eventr;
	std::vector<sf::CircleShape> list_eventc;

};

class Moving_ball: public sf::CircleShape{
	public:
		Moving_ball(): sf::CircleShape(){ 			
    		d1 = std::uniform_real_distribution<double>(MIN_BOUNCING_ANGLE_RL, MAX_BOUNCING_ANGLE_RL);
    		d2 = std::uniform_real_distribution<double>(MIN_BOUNCING_ANGLE_TD, MAX_BOUNCING_ANGLE_TD);
		}
		Moving_ball(float r, double angle, double velo): sf::CircleShape(r), ball_velocity(velo){
			ball_angle = angle * M_PI / 180.;
    		d1 = std::uniform_real_distribution<double>(MIN_BOUNCING_ANGLE_RL, MAX_BOUNCING_ANGLE_RL);
    		d2 = std::uniform_real_distribution<double>(MIN_BOUNCING_ANGLE_TD, MAX_BOUNCING_ANGLE_TD);
		}
		void set_angle(double a){ ball_angle = a * M_PI / 180.;}
		void set_velocity(double a){ ball_velocity = a;}
		void set_last_collision(SIDE a){ last_collision = a;}
		void set_new_trajectory(){

			if (last_collision == SIDE::RIGHT or last_collision == SIDE::LEFT){
				ball_angle += M_PI;
				ball_angle += d1(gen);
			}
			else if (last_collision == SIDE::TOP){
				ball_angle -= M_PI+M_PI/6;
				ball_angle += d2(gen);
			}

			else if (last_collision == SIDE::DOWN){
				ball_angle += M_PI+M_PI/6;
				ball_angle += d2(gen);
			}
			else{
				ball_angle += M_PI * d2(gen);
			}
			
		}
		void set_ball_is_lost(bool a){
			ball_is_lost = a;
		}
		bool get_ball_is_lost(){return ball_is_lost;}
		double get_velocity()const{return ball_velocity;};
		double get_angle()const{return ball_angle;};
	private:
		SIDE last_collision;
		bool ball_is_lost;
		std::default_random_engine gen;
		std::uniform_real_distribution<double> d1;
		std::uniform_real_distribution<double> d2;
		double ball_velocity;
		double ball_angle;
};

void Buffer_painter::paintElements(sf::RenderWindow &w){


	for(auto it = list_eventr.begin() ; it != list_eventr.end() ; ++it){
		w.draw(*it);
	}
	for(auto it = list_eventc.begin() ; it != list_eventc.end() ; ++it){
		w.draw(*it);
	}
	list_eventc.clear();
	list_eventr.clear();
}

sf::Vector2f compute_next_position(const Moving_ball &b, sf::Time delta_time){
	//if time = 0 return current position of ball
	if (!delta_time.asMilliseconds()){
		return b.getPosition();
	}

	double dist = delta_time.asSeconds() * b.get_velocity();

	return sf::Vector2f{static_cast<float>(b.getPosition().x + (cos(b.get_angle()) * dist)), 
						static_cast<float>(b.getPosition().y + (sin(b.get_angle()) * dist))};

}

bool is_in_collision(const Moving_ball &b, const sf::RectangleShape &r){
	
	// detecting which side of rectangle is the ball located
	//https://www.jeffreythompson.org/collision-detection/circle-rect.php
	sf::Vector2f pq = b.getPosition();
	pq.x += b.getRadius();
	pq.y += b.getRadius();

	if (b.getPosition().x + b.getRadius() < r.getPosition().x){
		pq.x = r.getPosition().x;
	}
	else if (b.getPosition().x + b.getRadius() > r.getPosition().x + r.getSize().x){
		pq.x = r.getPosition().x + r.getSize().x;
	}
	
	if (b.getPosition().y + b.getRadius() < r.getPosition().y){
		pq.y = r.getPosition().y;
	}
	else if (b.getPosition().y + b.getRadius() > r.getPosition().y + r.getSize().y){
		pq.y = r.getPosition().y + r.getSize().y;
	}

	sf::Vector2f pz;
	pz.x = b.getPosition().x + b.getRadius() - pq.x;
	pz.y = b.getPosition().y + b.getRadius() - pq.y;

	return hypot(pz.x, pz.y) <= b.getRadius();
}

bool is_in_collision(const sf::RectangleShape &r){
	if (r.getPosition().y < 0){
		return true;
	}
	else if (r.getPosition().y + r.getSize().y > WINDOW_Y){
		return true;
	}
	return false;
}

bool is_in_collision(Moving_ball &b){
	if (b.getPosition().y + b.getRadius() * 2 > WINDOW_Y){
		b.set_last_collision(SIDE::DOWN);
		return true;
	}
	else if (b.getPosition().y < 0){
		b.set_last_collision(SIDE::TOP);
		return true;
	}
	else if (b.getPosition().x < 0){
		b.set_last_collision(SIDE::LEFT);
		b.set_ball_is_lost(true);
		return true;
	}
	else if (b.getPosition().x + b.getRadius() * 2 > WINDOW_X){
		b.set_last_collision(SIDE::RIGHT);
		b.set_ball_is_lost(true);
		return true;
	}
	b.set_last_collision(SIDE::NO_COLLISION);
	return false;
}


int main()
{

	sf::RenderWindow window(sf::VideoMode(WINDOW_X, WINDOW_Y), "win");
	Buffer_painter elements;
	sf::Clock clock{};
	//declaration of graphics elements

	Moving_ball ball(BALL_RADIUS, BALL_START_ANGLE, BALL_START_VELOCITY);
	ball.setFillColor(sf::Color(0,0,0));
	ball.setPosition(BALL_START_POSITION_X, BALL_START_POSITION_Y);

	sf::RectangleShape rg(sf::Vector2f(35 ,100));
	rg.setFillColor(sf::Color(0, 150, 0));
	rg.setPosition(sf::Vector2f(0,0));

	sf::RectangleShape rd(sf::Vector2f(35 ,100));
	rd.setFillColor(sf::Color(150, 0, 150));
	rd.setPosition(sf::Vector2f(float(window.getSize().x - rd.getSize().x) , float(window.getSize().y - rd.getSize().y)));


	while(window.isOpen()){
		sf::Time elapsed = clock.restart();
		sf::Event event;
		sf::RectangleShape temp_rd(rd);
		sf::RectangleShape temp_rg(rg);
		while(window.pollEvent(event)){
			switch(event.type){

				case sf::Event::Closed:
					window.close();
				break;

				case sf::Event::KeyPressed:

				    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)){
						temp_rd.move(0, -20);
					}
					else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)){
						temp_rd.move(0, 20);
					}
					if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q)){
						temp_rg.move(0, -20);
					}
					else if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)){
						temp_rg.move(0, 20);
					}
					if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)){
						ball.set_new_trajectory();
					}
				break;
			}
		}

		//collision for ball
		Moving_ball temp_ball(ball);
		temp_ball.setPosition(compute_next_position(ball, elapsed));

		//collision ball with rectangles
		if (is_in_collision(temp_ball, rd)){
			ball.set_new_trajectory();
		}
		//collision ball with rectangles
		else if (is_in_collision(temp_ball, rg)){
			ball.set_new_trajectory();
		}
		//collision ball with sides
		else if (is_in_collision(temp_ball) && !temp_ball.get_ball_is_lost()){
			ball.set_new_trajectory();
		}
		else if (is_in_collision(temp_ball) && temp_ball.get_ball_is_lost()){
			//re throw ball
			ball.setPosition(BALL_START_POSITION_X, BALL_START_POSITION_Y);
		}
		//updating position of ball if no events
		else{
			ball.setPosition(temp_ball.getPosition());
		}


		//collision for rectangles and sides
		if (!is_in_collision(temp_rd)){
			rd.setPosition(temp_rd.getPosition());
		}
		if (!is_in_collision(temp_rg)){
			rg.setPosition(temp_rg.getPosition());
		}




		//elements to paint
		elements.push_event(rg);
		elements.push_event(rd);
		elements.push_event(ball);


		window.clear(sf::Color::White);
		elements.paintElements(window);
		window.display();

	}

	return 0;
}
