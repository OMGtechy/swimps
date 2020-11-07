#include "swimps-test.h"
#include "swimps-container.h"

SCENARIO("swimps::container::span", "[swimps-container]") {
    GIVEN("A target buffer of 7 bytes, where each byte is initialised as its index.") {
        char targetBuffer[] = { 0, 1, 2, 3, 4, 5, 6 };
        static_assert(sizeof targetBuffer == 7);

        WHEN("A span is created to miss the first and last bytes.") {
            auto span = swimps::container::Span(targetBuffer + 1, 5);

            THEN("The value at each index is offset by 1.") {
                REQUIRE(span[0] == 1);
                REQUIRE(span[1] == 2);
                REQUIRE(span[2] == 3);
                REQUIRE(span[3] == 4);
                REQUIRE(span[4] == 5);
            }

            THEN("The remaining size of the span is 5") {
                REQUIRE(span.remaining_size() == 5);
            }

            THEN("The target buffer is unchanged.") {
                REQUIRE(targetBuffer[0] == 0);
                REQUIRE(targetBuffer[1] == 1);
                REQUIRE(targetBuffer[2] == 2);
                REQUIRE(targetBuffer[3] == 3);
                REQUIRE(targetBuffer[4] == 4);
                REQUIRE(targetBuffer[5] == 5);
                REQUIRE(targetBuffer[6] == 6);
            }

            AND_WHEN("Each byte in the span has a unique value written to it.") {
                span[0] = 42;
                span[1] = 17;
                span[2] = 50;
                span[3] = 11;
                span[4] = 99;

                THEN("The target buffer is changed at an offset of 1.") {
                    REQUIRE(targetBuffer[1] == 42);
                    REQUIRE(targetBuffer[2] == 17);
                    REQUIRE(targetBuffer[3] == 50);
                    REQUIRE(targetBuffer[4] == 11);
                    REQUIRE(targetBuffer[5] == 99);
                }

                THEN("Reading back from the span gives the assigned values.") {
                    REQUIRE(span[0] == 42);
                    REQUIRE(span[1] == 17);
                    REQUIRE(span[2] == 50);
                    REQUIRE(span[3] == 11);
                    REQUIRE(span[4] == 99);
                }

                THEN("The values outside the span are unchanged.") {
                    REQUIRE(targetBuffer[0] == 0);
                    REQUIRE(targetBuffer[6] == 6);
                }

                AND_WHEN("The post-increment operator is called.") {
                    {
                        const auto returnedSpan = span++;

                        THEN("The returned value hasn't changed; this is *post* increment.") {
                            REQUIRE(returnedSpan.remaining_size() == 5);
                            REQUIRE(returnedSpan[0] == 42);
                            REQUIRE(returnedSpan[1] == 17);
                            REQUIRE(returnedSpan[2] == 50);
                            REQUIRE(returnedSpan[3] == 11);
                            REQUIRE(returnedSpan[4] == 99);
                        }
                    }

                    THEN("The remaining size of the original span goes down by 1.") {
                        REQUIRE(span.remaining_size() == 4);
                    }

                    THEN("The span now reads at an offset of 2.") {
                        REQUIRE(span[0] == 17);
                        REQUIRE(span[1] == 50);
                        REQUIRE(span[2] == 11);
                        REQUIRE(span[3] == 99);
                    }

                    THEN("The target buffer is unchanged.") {
                        REQUIRE(targetBuffer[0] == 0);  
                        REQUIRE(targetBuffer[1] == 42);
                        REQUIRE(targetBuffer[2] == 17);
                        REQUIRE(targetBuffer[3] == 50);
                        REQUIRE(targetBuffer[4] == 11);
                        REQUIRE(targetBuffer[5] == 99);
                        REQUIRE(targetBuffer[6] == 6);
                    }

                    AND_WHEN("The pre-increment operator is called.") {
                        {
                            const auto returnedSpan = ++span;

                            THEN("The returned and span is equivalent; this is *pre* increment.") {
                                REQUIRE(returnedSpan == span);
                            }
                        }

                        THEN("The remaining size of the original span goes down by 1.") {
                            REQUIRE(span.remaining_size() == 3);
                        }

                        AND_WHEN("Each valid index is written to.") {
                            span[0] = 81;
                            span[1] = 82;
                            span[2] = 83;

                            THEN("Reading back from the span gives the assigned values.") {
                                REQUIRE(span[0] == 81);
                                REQUIRE(span[1] == 82);
                                REQUIRE(span[2] == 83);
                            }

                            THEN("The target buffer is changed at an offset of 3.") {
                                REQUIRE(targetBuffer[3] == 81);  
                                REQUIRE(targetBuffer[4] == 82);
                                REQUIRE(targetBuffer[5] == 83);
                            }

                            THEN("The values outside the span are unchanged.") {
                                REQUIRE(targetBuffer[0] == 0);  
                                REQUIRE(targetBuffer[1] == 42);
                                REQUIRE(targetBuffer[2] == 17);
                                REQUIRE(targetBuffer[6] == 6);
                            }

                            AND_WHEN("The post-decrement operator is called.") {
                                {
                                    const auto returnedSpan = span--;

                                    THEN("The return value hasn't changed; this is *post* decrement.") {
                                        REQUIRE(returnedSpan.remaining_size() == 3);
                                        REQUIRE(returnedSpan[0] == 81);
                                        REQUIRE(returnedSpan[1] == 82);
                                        REQUIRE(returnedSpan[2] == 83);
                                    }
                                }

                                THEN("The remaining size of the original span increases by 1.") {
                                    REQUIRE(span.remaining_size() == 4);
                                }

                                THEN("The target buffer is unchanged.") {
                                    REQUIRE(targetBuffer[0] == 0);  
                                    REQUIRE(targetBuffer[1] == 42);
                                    REQUIRE(targetBuffer[2] == 17);
                                    REQUIRE(targetBuffer[3] == 81);  
                                    REQUIRE(targetBuffer[4] == 82);
                                    REQUIRE(targetBuffer[5] == 83);
                                    REQUIRE(targetBuffer[6] == 6);
                                }

                                THEN("The span reads at an offset of 2.") {
                                    REQUIRE(span[0] == 17);
                                    REQUIRE(span[1] == 81);  
                                    REQUIRE(span[2] == 82);
                                    REQUIRE(span[3] == 83);
                                    REQUIRE(span[4] == 6);
                                }

                                AND_WHEN("The pre-decrement operator is called.") {
                                    {
                                        const auto returnedSpan = --span;

                                        THEN("The returned and new span are equivalent; this is *pre* decrement.") {
                                            REQUIRE(returnedSpan == span);
                                        }
                                    }

                                    THEN("The remaining size of the original span increases by 1.") {
                                        REQUIRE(span.remaining_size() == 5);
                                    }

                                    THEN("The span reads at an offset of 1.") {
                                        REQUIRE(span[0] == 42);
                                        REQUIRE(span[1] == 17);
                                        REQUIRE(span[2] == 81);  
                                        REQUIRE(span[3] == 82);
                                        REQUIRE(span[4] == 83);
                                    }

                                    THEN("The target buffer is unchanged.") {
                                        REQUIRE(targetBuffer[0] == 0);  
                                        REQUIRE(targetBuffer[1] == 42);
                                        REQUIRE(targetBuffer[2] == 17);
                                        REQUIRE(targetBuffer[3] == 81);  
                                        REQUIRE(targetBuffer[4] == 82);
                                        REQUIRE(targetBuffer[5] == 83);
                                        REQUIRE(targetBuffer[6] == 6);
                                    }

                                    AND_WHEN("Each valid index is written to.") {
                                        span[0] = 10;
                                        span[1] = 11;
                                        span[2] = 12;
                                        span[3] = 13;
                                        span[4] = 14;

                                        THEN("The target buffer is changed at an offset of 1.") {
                                            REQUIRE(targetBuffer[1] == 10);
                                            REQUIRE(targetBuffer[2] == 11);
                                            REQUIRE(targetBuffer[3] == 12);  
                                            REQUIRE(targetBuffer[4] == 13);
                                            REQUIRE(targetBuffer[5] == 14);
                                        }

                                        THEN("The values outside of the target buffer are unchanged.") {
                                            REQUIRE(targetBuffer[0] == 0);  
                                            REQUIRE(targetBuffer[6] == 6);
                                        } 

                                        THEN("Reading back from the span gives the assigned values.") {
                                            REQUIRE(span[0] == 10);
                                            REQUIRE(span[1] == 11);
                                            REQUIRE(span[2] == 12);
                                            REQUIRE(span[3] == 13);
                                            REQUIRE(span[4] == 14);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    GIVEN("A target buffer of 8 zero-initialised bytes.") {
        char targetBuffer[8] = { 0 };

        WHEN("A span is created over it.") {
            swimps::container::Span span(targetBuffer);

            THEN("All bytes read back as 0.") {
                REQUIRE(span[0] == 0);
                REQUIRE(span[1] == 0);
                REQUIRE(span[2] == 0);
                REQUIRE(span[3] == 0);
                REQUIRE(span[4] == 0);
                REQUIRE(span[5] == 0);
                REQUIRE(span[6] == 0);
                REQUIRE(span[7] == 0);
            }

            AND_WHEN("Each byte has its respective index written to it.") {
                span[0] = 0;
                span[1] = 1;
                span[2] = 2;
                span[3] = 3;
                span[4] = 4;
                span[5] = 5;
                span[6] = 6;
                span[7] = 7;

                THEN("The original size of the span is correct.") {
                    REQUIRE(span.original_size() == 8);
                }

                THEN("The remaining size of the span is correct.") {
                    REQUIRE(span.remaining_size() == 8);
                }

                THEN("Each byte reads back correctly.") {
                    REQUIRE(span[0] == 0);
                    REQUIRE(span[1] == 1);
                    REQUIRE(span[2] == 2);
                    REQUIRE(span[3] == 3);
                    REQUIRE(span[4] == 4);
                    REQUIRE(span[5] == 5);
                    REQUIRE(span[6] == 6);
                    REQUIRE(span[7] == 7);
                }

                AND_WHEN("operator+ is called with an offset of 1.") {
                    const auto addedSpan = span + 1;

                    THEN("The added span is not equivalent to the original.") {
                        REQUIRE(addedSpan != span);
                        REQUIRE(span != addedSpan);
                        REQUIRE(!(addedSpan == span));
                        REQUIRE(!(span == addedSpan));
                    }

                    THEN("The original span hasn't changed.") {
                        REQUIRE(span[0] == 0);
                        REQUIRE(span[1] == 1);
                        REQUIRE(span[2] == 2);
                        REQUIRE(span[3] == 3);
                        REQUIRE(span[4] == 4);
                        REQUIRE(span[5] == 5);
                        REQUIRE(span[6] == 6);
                        REQUIRE(span[7] == 7);
                        REQUIRE(span.remaining_size() == 8);
                        REQUIRE(span.original_size() == 8);
                    }

                    THEN("The added span is at the correct offset.") {
                        REQUIRE(addedSpan[0] == 1);
                        REQUIRE(addedSpan[1] == 2);
                        REQUIRE(addedSpan[2] == 3);
                        REQUIRE(addedSpan[3] == 4);
                        REQUIRE(addedSpan[4] == 5);
                        REQUIRE(addedSpan[5] == 6);
                        REQUIRE(addedSpan[6] == 7);
                    }

                    THEN("The remaining size of the added span is correct.") {
                        REQUIRE(addedSpan.remaining_size() == 7);
                    }

                    THEN("The original size of the added span is correct.") {
                        REQUIRE(span.original_size() == 8);
                    }

                    AND_WHEN("operator- is called on the added span with an offset of 1.") {
                        const auto subtractedSpan = addedSpan - 1;

                        THEN("The subtracted span is equivalent to the original.") {
                            REQUIRE(subtractedSpan == span);
                            REQUIRE(span == subtractedSpan);
                            REQUIRE(!(subtractedSpan != span));
                            REQUIRE(!(span != subtractedSpan));
                        }

                        THEN("The added span is not equivalent to the subtracted span.") {
                            REQUIRE(subtractedSpan != addedSpan);
                            REQUIRE(addedSpan != subtractedSpan);
                            REQUIRE(!(subtractedSpan == addedSpan));
                            REQUIRE(!(addedSpan == subtractedSpan));
                        }

                        THEN("The original span hasn't changed.") {
                            REQUIRE(span[0] == 0);
                            REQUIRE(span[1] == 1);
                            REQUIRE(span[2] == 2);
                            REQUIRE(span[3] == 3);
                            REQUIRE(span[4] == 4);
                            REQUIRE(span[5] == 5);
                            REQUIRE(span[6] == 6);
                            REQUIRE(span[7] == 7);
                            REQUIRE(span.remaining_size() == 8);
                            REQUIRE(span.original_size() == 8);
                        }

                        THEN("The added span hasn't changed.") {
                            REQUIRE(addedSpan[0] == 1);
                            REQUIRE(addedSpan[1] == 2);
                            REQUIRE(addedSpan[2] == 3);
                            REQUIRE(addedSpan[3] == 4);
                            REQUIRE(addedSpan[4] == 5);
                            REQUIRE(addedSpan[5] == 6);
                            REQUIRE(addedSpan[6] == 7);
                            REQUIRE(addedSpan.remaining_size() == 7);
                            REQUIRE(addedSpan.original_size() == 8);
                        }

                        THEN("The subtracted span is at an offset of 0.") {
                            REQUIRE(subtractedSpan[0] == 0);
                            REQUIRE(subtractedSpan[1] == 1);
                            REQUIRE(subtractedSpan[2] == 2);
                            REQUIRE(subtractedSpan[3] == 3);
                            REQUIRE(subtractedSpan[4] == 4);
                            REQUIRE(subtractedSpan[5] == 5);
                            REQUIRE(subtractedSpan[6] == 6);
                            REQUIRE(subtractedSpan[7] == 7);
                        }

                        THEN("The remaining size of the subtracted span is correct.") {
                            REQUIRE(subtractedSpan.remaining_size() == 8);
                        }

                        THEN("The original size of the subtracted span is correct.") {
                            REQUIRE(subtractedSpan.original_size() == 8);
                        }
                    }
                }

                AND_WHEN("operator+ is called with an offset of 3.") {
                    const auto addedSpan = span + 3;

                    THEN("The added span is not equivalent to the original.") {
                        REQUIRE(addedSpan != span);
                        REQUIRE(span != addedSpan);
                        REQUIRE(!(addedSpan == span));
                        REQUIRE(!(span == addedSpan));
                    }

                    THEN("The original span hasn't changed.") {
                        REQUIRE(span[0] == 0);
                        REQUIRE(span[1] == 1);
                        REQUIRE(span[2] == 2);
                        REQUIRE(span[3] == 3);
                        REQUIRE(span[4] == 4);
                        REQUIRE(span[5] == 5);
                        REQUIRE(span[6] == 6);
                        REQUIRE(span[7] == 7);
                        REQUIRE(span.remaining_size() == 8);
                        REQUIRE(span.original_size() == 8);
                    }

                    THEN("The added span is the correct offset.") {
                        REQUIRE(addedSpan[0] == 3);
                        REQUIRE(addedSpan[1] == 4);
                        REQUIRE(addedSpan[2] == 5);
                        REQUIRE(addedSpan[3] == 6);
                        REQUIRE(addedSpan[4] == 7);
                    }

                    THEN("The remaining size of the added span is correct.") {
                        REQUIRE(addedSpan.remaining_size() == 5);
                    }

                    THEN("The original size of the added span is correct.") {
                        REQUIRE(addedSpan.original_size() == 8);
                    }

                    AND_WHEN("operator- is called on the added span with an offset of 2.") {
                        const auto subtractedSpan = addedSpan - 2;

                        THEN("The substracted span is not equivalent to the original.") {
                            REQUIRE(subtractedSpan != span);
                            REQUIRE(span != subtractedSpan);
                            REQUIRE(!(subtractedSpan == span));
                            REQUIRE(!(span == subtractedSpan));
                        }

                        THEN("The original span hasn't changed.") {
                            REQUIRE(span[0] == 0);
                            REQUIRE(span[1] == 1);
                            REQUIRE(span[2] == 2);
                            REQUIRE(span[3] == 3);
                            REQUIRE(span[4] == 4);
                            REQUIRE(span[5] == 5);
                            REQUIRE(span[6] == 6);
                            REQUIRE(span[7] == 7);
                            REQUIRE(span.remaining_size() == 8);
                            REQUIRE(span.original_size() == 8);
                        }

                        THEN("The added span hasn't changed.") {
                            REQUIRE(addedSpan[0] == 3);
                            REQUIRE(addedSpan[1] == 4);
                            REQUIRE(addedSpan[2] == 5);
                            REQUIRE(addedSpan[3] == 6);
                            REQUIRE(addedSpan[4] == 7);
                            REQUIRE(addedSpan.remaining_size() == 5);
                            REQUIRE(addedSpan.original_size() == 8);
                        }

                        THEN("The subtracted span is at the correct offset.") {
                            REQUIRE(subtractedSpan[0] == 1);
                            REQUIRE(subtractedSpan[1] == 2);
                            REQUIRE(subtractedSpan[2] == 3);
                            REQUIRE(subtractedSpan[3] == 4);
                            REQUIRE(subtractedSpan[4] == 5);
                            REQUIRE(subtractedSpan[5] == 6);
                            REQUIRE(subtractedSpan[6] == 7);
                        }

                        THEN("The remaining size of the subtracted span is correct.") {
                            REQUIRE(subtractedSpan.remaining_size() == 7);
                        }

                        THEN("The original size of the subtracted span is correct.") {
                            REQUIRE(subtractedSpan.original_size() == 8);
                        }
                    }
                }

                AND_WHEN("A copy of the original span is made.") {
                    auto movingSpan = span;

                    THEN("It is equivalent to the original.") {
                        REQUIRE(movingSpan == span);
                        REQUIRE(span == movingSpan);
                        REQUIRE(!(movingSpan != span));
                        REQUIRE(!(span != movingSpan));
                    }

                    AND_WHEN("operator+= is used to offset it by 6.") {
                        movingSpan += 6;

                        THEN("The span is not equivalent to the original.") {
                            REQUIRE(movingSpan != span);
                            REQUIRE(span != movingSpan);
                            REQUIRE(!(movingSpan == span));
                            REQUIRE(!(span == movingSpan));
                        }

                        THEN("The span is offset by accordingly.") {
                            REQUIRE(movingSpan[0] == 6);
                            REQUIRE(movingSpan[1] == 7);
                        }

                        THEN("The remaining size is correct.") {
                            REQUIRE(movingSpan.remaining_size() == 2);
                        }

                        THEN("The original size is correct.") {
                            REQUIRE(movingSpan.original_size() == 8);
                        }

                        THEN("The original span is unchanged.") {
                            REQUIRE(span[0] == 0);
                            REQUIRE(span[1] == 1);
                            REQUIRE(span[2] == 2);
                            REQUIRE(span[3] == 3);
                            REQUIRE(span[4] == 4);
                            REQUIRE(span[5] == 5);
                            REQUIRE(span[6] == 6);
                            REQUIRE(span[7] == 7);
                            REQUIRE(span.remaining_size() == 8);
                            REQUIRE(span.original_size() == 8);
                        }

                        AND_WHEN("operator-= is used to decrease the offset by 1.") {
                            movingSpan -= 1;

                            THEN("The span is not equivalent to the original.") {
                                REQUIRE(movingSpan != span);
                                REQUIRE(span != movingSpan);
                                REQUIRE(!(movingSpan == span));
                                REQUIRE(!(span == movingSpan));
                            }

                            THEN("The span is offset by 1 less than before.") {
                                REQUIRE(movingSpan[0] == 5);
                                REQUIRE(movingSpan[1] == 6);
                                REQUIRE(movingSpan[2] == 7);
                            }

                            THEN("The remaining size is correct.") {
                                REQUIRE(movingSpan.remaining_size() == 3);
                            }

                            THEN("The original size is correct.") {
                                REQUIRE(movingSpan.original_size() == 8);
                            }

                            THEN("The original span is unchanged.") {
                                REQUIRE(span[0] == 0);
                                REQUIRE(span[1] == 1);
                                REQUIRE(span[2] == 2);
                                REQUIRE(span[3] == 3);
                                REQUIRE(span[4] == 4);
                                REQUIRE(span[5] == 5);
                                REQUIRE(span[6] == 6);
                                REQUIRE(span[7] == 7);
                                REQUIRE(span.remaining_size() == 8);
                                REQUIRE(span.original_size() == 8);
                            }

                            AND_WHEN("operator-= is used to decrease the offset by 5.") {
                                movingSpan -= 5;

                                THEN("The span is now equivalent to the original again.") {
                                    REQUIRE(!(movingSpan != span));
                                    REQUIRE(!(span != movingSpan));
                                    REQUIRE(movingSpan == span);
                                    REQUIRE(span == movingSpan);
                                }
                            }
                        }
                    }
                }
            }

            WHEN("Zero is added to the span.") {
                const auto newSpan = span + 0;

                THEN("It is equivalent to the previous span.") {
                    REQUIRE(newSpan == span);
                }
            }

            WHEN("Zero is subtracted from the span.") {
                const auto newSpan = span - 0;

                THEN("It is equivalent to the previous span.") {
                    REQUIRE(newSpan == span);
                }
            }

            WHEN("A copy of the span is made.") {
                auto newSpan = span;

                AND_WHEN("operator += is used to increase its offset by 0.") {
                    newSpan += 0;

                    THEN("It is equivalent to the original span.") {
                        REQUIRE(newSpan == span);
                    }
                }

                AND_WHEN("operator -= is used to decrease its offset by 0.") {
                    newSpan -= 0;

                    THEN("It is equivalent to the original span.") {
                        REQUIRE(newSpan == span);
                    }
                }
            }
        }
    }
}
