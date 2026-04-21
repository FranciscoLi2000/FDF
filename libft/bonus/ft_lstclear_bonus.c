#include "libft_bonus.h"

void	ft_lstclear(t_list **lst, void (*del)(void *))
{
t_list *next;

if (!lst || !del)
return ;
while (*lst)
{
next = (*lst)->next;
ft_lstdelone(*lst, del);
*lst = next;
}
}
