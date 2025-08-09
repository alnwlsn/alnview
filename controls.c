// case SDL_MOUSEBUTTONDOWN:
//           if (e.button.button == SDL_BUTTON_LEFT) {
//             dragging = 1;
//             drag_mouse_x = e.button.x;
//             drag_mouse_y = e.button.y;
//           }
//           break;
//         case SDL_MOUSEBUTTONUP:
//           if (e.button.button == SDL_BUTTON_LEFT) {
//             dragging = 0;
//           }
//           break;
//         case SDL_MOUSEMOTION:
//           if (dragging) {
//             // drag canvas around on screen
//             float sdx = (e.motion.x - drag_mouse_x) / cv.z;
//             float sdy = -(e.motion.y - drag_mouse_y) / cv.z;
//             float cdx = sdx * cos(-cv.r * M_PI / 180) - sdy * sin(-cv.r * M_PI / 180);  // proper coordinate rotation
//             float cdy = sdy * cos(-cv.r * M_PI / 180) + sdx * sin(-cv.r * M_PI / 180);
//             cv.x -= cdx;
//             cv.y -= cdy;
//             drag_mouse_x = e.motion.x;
//             drag_mouse_y = e.motion.y;
//           }
//           break;
//         case SDL_MOUSEWHEEL: {
//           // zoom adjustment centered on mouse
//           // find old and new position; offset view accordingly
//           float scx1 = (mouse_screen_x / cv.z);
//           float scy1 = (mouse_screen_y / cv.z);
//           if (e.wheel.y > 0)
//             cv.z *= 1.1f;
//           else if (e.wheel.y < 0)
//             cv.z /= 1.1f;
//           float scx2 = (mouse_screen_x / cv.z);
//           float scy2 = (mouse_screen_y / cv.z);
//           float sdx = scx2 - scx1;
//           float sdy = scy2 - scy1;
//           float cdx = sdx * cos(-cv.r * M_PI / 180) - sdy * sin(-cv.r * M_PI / 180);  // proper coordinate rotation
//           float cdy = sdy * cos(-cv.r * M_PI / 180) + sdx * sin(-cv.r * M_PI / 180);
//           cv.x -= cdx;
//           cv.y -= cdy;
//         } break;
//         case SDL_KEYDOWN:
//           switch (e.key.keysym.sym) {
//             case SDLK_BACKSLASH:
//               animation = 1;
//               animation_step = 0;
//               break;
//             case SDLK_5:
//               cv.r = 0;
//               cv.z = 1.0f;
//               cv.x = 128;
//               cv.y = -128;
//               break;
//             case SDLK_6:
//               cv.r = 22.5;
//               cv.z = 1.0f;
//               cv.x = 128;
//               cv.y = -128;
//               break;
//             case SDLK_7:
//               cv.r = 45;
//               cv.z = 1.0f;
//               cv.x = 128;
//               cv.y = -128;
//               break;
//             case SDLK_8:
//               cv.r = -45;
//               cv.z = 1.0f;
//               cv.x = 0;
//               cv.y = 0;
//               break;
//             case SDLK_9:
//               waypt[0] = cv;
//               break;
//             case SDLK_0:
//               waypt[1] = cv;
//               break;
//             case SDLK_o:
//               cv = waypt[0];
//               break;
//             case SDLK_p:
//               cv = waypt[1];
//               break;
//             case SDLK_1:
//               float dr = 15.0f;

//               cv.r -= dr;
//               float sdx = canvas_rx-cv.x;
//               float sdy = canvas_ry-cv.y;
//               float cx = sdx * cos(dr * M_PI / 180) - sdy * sin(dr * M_PI / 180) + cv.x; 
//               float cy = sdy * cos(dr * M_PI / 180) + sdx * sin(dr * M_PI / 180) + cv.y;
//               // float cdx = -canvas_rx+cv.x;
//               // float cdy = -canvas_ry+cv.y;
//               ax = cx;
//               ay = cy;
//               bx = cv.x-(cx-canvas_rx);
//               by = cv.y-(cy-canvas_ry);
//               printf("%.1f, %.1f\n", ax, ay);
              
//               cv.x = bx;
//               cv.y = by;
//               break;
//             case SDLK_2:
//               cv.r += 15.0f;
//               break;
//             case SDLK_q:
//               break;
//             case SDLK_w:
//               cv.r = 0.0f;
//               cv.x = 0;
//               cv.y = 0;
//               cv.z = 1.0;
//               break;
//             case SDLK_r:  // pick new point to rotate world around
//             {
//               canvas_rx = mouse_canvas_x;
//               canvas_ry = mouse_canvas_y;
//             } break;
//             case SDLK_e:
//               break;
//             case SDLK_i:
//               cv.y += 1;
//               break;
//             case SDLK_k:
//               cv.y -= 1;
//               break;
//             case SDLK_j:
//               cv.x -= 1;
//               break;
//             case SDLK_l:
//               cv.x += 1;
//               break;
//           }
//           break;